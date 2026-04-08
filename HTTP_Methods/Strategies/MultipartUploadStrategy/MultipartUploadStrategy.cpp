#include "MultipartUploadStrategy.hpp"

MultipartUploadStrategy::MultipartUploadStrategy(ClientSocket *socketIO, Routing *routing)
    : _socketIO(socketIO), _router(routing), _request(routing->GetRequest()),
      _chunkedData(routing->GetChunkedData()), _multipartData(routing->GetMultipartData())
{
    if (_request.isChunkedTransferEncoding() == false)
        _contentLen = _request.getcontentlen();
    else
        _contentLen = -1;
    _maxSize = _request.getMaxBodySize();
    _totalSize = 0;
    _buffer = (char *)(_request.getBody().c_str());
    _len = _request.getBody().length();
    _subBufferUploadStrategy = new SubBufferUploadStrategy();
    _decodeData();
    if (_status < eComplete) {
        _buffer = NULL;
        return;
    }
    _uploadBuffers(_multipartData.GetParts());
    if (_multipartData.GetStatus() == MultipartData::eComplete 
        && _status == eContinue && 
        (_request.isChunkedTransferEncoding() == false || 
        _chunkedData.GetStatus() == ChunkedData::eComplete))
        _status = eComplete;
    _buffer = NULL;
    if (_status == eContinue)
        _buffer = Utility::GetBuffer();
}

MultipartUploadStrategy::~MultipartUploadStrategy()
{
    delete _subBufferUploadStrategy;
    if (_buffer)
        Utility::ReleaseBuffer(_buffer);
}

void MultipartUploadStrategy::_uploadBuffer(FormData *formData)
{
    string filename = formData->filename;
    _router->AddPath(filename);
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
    {
        _status = eOpenFileError;
        return;
    }
    _router->addFileUploaded(filename);
    if (!formData->bodyPart.empty())
    {
        _subBufferUploadStrategy->SetBuffer(&formData->bodyPart[0], formData->bodyPart.length(), fd);
        int subStatus = _subBufferUploadStrategy->Execute();
        if (subStatus != AStrategy::eComplete)
            _status = eWriteError;
    }
    if (formData->bodyLen > 0)
    {
        _subBufferUploadStrategy->SetBuffer(_buffer + formData->bodyStart, formData->bodyLen, fd);
        int subStatus = _subBufferUploadStrategy->Execute();
        if (subStatus != AStrategy::eComplete)
            _status = eWriteError;
    }
    close(fd);
}

void MultipartUploadStrategy::_uploadBuffers(queue<FormData> &formDataQueue)
{
    DDEBUG("MultipartUploadStrategy") << "1-_uploadBuffers(): formDataQueue size=" << formDataQueue.size();
    while (!formDataQueue.empty())
    {
        FormData &formData = formDataQueue.front();
        _uploadBuffer(&formData);
        formDataQueue.pop();
    }
}

void MultipartUploadStrategy::_decodeData()
{
    DDEBUG("MultipartUploadStrategy") << "1-_decodeData(): _len=" << _len;
    if (_request.isChunkedTransferEncoding())
    {
        _chunkResult = _chunkedData.Feed(_buffer, _len);
        if (_chunkedData.GetStatus() == ChunkedData::eError)
        {
            DDEBUG("MultipartUploadStrategy") << "2-_decodeData(): ChunkedData error";
            _status = eChunkedError;
            return;
        }
        _totalSize += _chunkResult.decodedLen;

        _multipartData.Feed(_buffer, _chunkResult.decodedLen);
    }
    else
    {
        _totalSize += _len;
        _multipartData.Feed(_buffer, _len);
    }
    if ((_chunkedData.GetStatus() == ChunkedData::eComplete && _multipartData.GetStatus() != MultipartData::eComplete) || _multipartData.GetStatus() == MultipartData::eError)
    {
        _status = eMultipartError;
        DDEBUG("MultipartUploadStrategy")
            << "3-_decodeData(): MultipartData error, "
            << "_chunkedData status=" << _chunkedData.GetStatus()
            << ", _multipartData status=" << _multipartData.GetStatus();
    }
    if (_totalSize > _maxSize) {
        _status = eMaxBodySizeExceeded;
    }
    if (_contentLen != -1 && _totalSize == (size_t)_contentLen && _multipartData.GetStatus() != MultipartData::eComplete)
    {
        _status = eMultipartError;
        DDEBUG("MultipartUploadStrategy") << "4-_decodeData(): Content-Length mismatch with multipart data";
    }
}

bool MultipartUploadStrategy::_needToRead()
{
    if (_request.isChunkedTransferEncoding())
    {
        if (_chunkedData.GetStatus() == ChunkedData::eComplete)
            return false;
    }
    else if (_multipartData.GetStatus() == MultipartData::eComplete)
        return false;
    return true;
}

void MultipartUploadStrategy::_updateStatus()
{
    if (_status != eContinue)
        return;
    if (_request.isChunkedTransferEncoding())
    {
        if (_chunkedData.GetStatus() == ChunkedData::eComplete)
            _status = eComplete;
    }
    else if (_multipartData.GetStatus() == MultipartData::eComplete)
        _status = eComplete;
}

int MultipartUploadStrategy::Execute()
{
    DDEBUG("MultipartUploadStrategy") << "1-Execute()";
    if (_status != eContinue)
        return _status;
    _len = 0;
    if (_needToRead())
    {
        size_t toRead = BUF_SIZE;
        if (_contentLen != -1 && _contentLen - _totalSize < toRead)
            toRead = _contentLen - _totalSize;
        _len = read(_socketIO->GetFd(), _buffer, toRead);
        DDEBUG("MultipartUploadStrategy") << "2-Execute(): read " << _len << " bytes";
        if (_len <= 0)
        {
            _status = eReadError;
            return _status;
        }
        else
            _socketIO->UpdateTime();
    }
    if (_len > 0)
    {
        _decodeData();
        DDEBUG("MultipartUploadStrategy") << "4-Execute(): _status=" << _status;
        if (_status != eContinue)
            return _status;
    }
    _uploadBuffers(_multipartData.GetParts());
    _updateStatus();
    return _status;
}
