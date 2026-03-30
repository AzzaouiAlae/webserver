#include "MulipartUploadStrategy.hpp"

MulipartUploadStrategy::MulipartUploadStrategy(SocketIO *socketIO, Routing *routing)
    : _socketIO(socketIO), _routing(routing), _request(routing->GetRequest()), 
    _chunkedData(routing->GetChunkedData()), _multipartData(routing->GetMultipartData()), 
    _str(routing->GetRequest().getBody()), _chunkStatus(0), _multipartStatus(0)
{
    _buffer = Utility::GetBuffer();
    _subBufferUploadStrategy = new SubBufferUploadStrategy();
    _boundery = "--" + _request.getMultipartBoundary();
    _decodeData();
}

MulipartUploadStrategy::~MulipartUploadStrategy()
{
    if (_subBufferUploadStrategy)
        delete _subBufferUploadStrategy;
    Utility::ReleaseBuffer(_buffer);
}

void MulipartUploadStrategy::_uploadBuffer(FormData *formData)
{
    if (formData == NULL || formData->size == 0)
        return;
    DDEBUG("MulipartUploadStrategy") << "1-_uploadBuffer(): formData name=" << formData->name 
            << ", filename=" << formData->filename << ", contentType=" << formData->contentType
            << ", size=" << formData->size
            << ", buffer=\n" << string(formData->data(), formData->size);
    _routing->AddPath(formData->filename);
    int fd = formData->openFile();
    if (fd < 0)
    {
        _status = eOpenFileError;
        return;
    }
    _subBufferUploadStrategy->SetBuffer(formData->data(), formData->size, fd);
    int subStatus = _subBufferUploadStrategy->Execute();
    if (subStatus != AStrategy::eComplete)
        _status = eWriteError;
}

void MulipartUploadStrategy::_uploadBuffers(queue<FormData *> &formDataQueue)
{
    DDEBUG("MulipartUploadStrategy") << "1-_uploadBuffers(): formDataQueue size=" << formDataQueue.size();
    FormData *formData;
    while (!formDataQueue.empty())
    {
        formData = formDataQueue.front();
        formDataQueue.pop();
        _uploadBuffer(formData);
        if (_status != eContinue)
            return;
    }
    formData = _multipartData.getCurrentFormData();
    _uploadBuffer(formData);
    if (_status != eContinue)
        return;
    size_t lastIdx = _multipartData.getLastIndex();
    _str.erase(0, lastIdx);
    _multipartData.resetIndex();
    if (_request.isChunkedTransferEncoding())
        _chunkedData.SizeWritting(lastIdx + 1);
}

void MulipartUploadStrategy::_decodeData()
{
    DDEBUG("MulipartUploadStrategy") << "1-_decodeData(): _str:\n" << _str;
    if (_request.isChunkedTransferEncoding())
        _chunkStatus = _chunkedData.UnchunkData(_str);
    DDEBUG("MulipartUploadStrategy") << "2-_decodeData(): _chunkStatus=" << _chunkStatus;
    if (_chunkStatus == ChunkedData::eError)
    {
        _status = eChunkedError;
        return;
    }
    _multipartStatus = _multipartData.Parse(_str, _boundery);
    DDEBUG("MulipartUploadStrategy") << "3-_decodeData(): _multipartStatus=" << _multipartStatus;
    if ((_chunkStatus == ChunkedData::eComplete && _multipartStatus != MultipartData::eComplete) 
        || _multipartStatus == MultipartData::eError)
        _status = eMultipartError;
}

int MulipartUploadStrategy::Execute()
{
    DDEBUG("MulipartUploadStrategy") << "1-Execute()";
    if (_status != eContinue)
        return _status;
    
    ssize_t readLen = 0;
    if (_multipartStatus != MultipartData::eComplete) {
        readLen = read(_socketIO->GetFd(), _buffer, BUF_SIZE);
        DDEBUG("MulipartUploadStrategy") << "2-Execute(): read " << readLen << " bytes";
        if (readLen <= 0) {
            _status = eReadError;
            return _status;
        }
    }
    if (readLen > 0) {
        _str.append(_buffer, readLen);
        _decodeData();
        DDEBUG("MulipartUploadStrategy") << "4-Execute(): _status=" << _status;
        if (_status != eContinue)
            return _status;
    }
    _uploadBuffers(_multipartData.getFormData());
    if (_multipartStatus == MultipartData::eComplete)
        _status = eComplete;
    return _status;
}
