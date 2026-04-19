#include "UploadStrategy.hpp"

UploadStrategy::UploadStrategy(Routing *router, ClientSocket *sock) : 
_router(router), _request(_router->GetRequest()), _path(_router->GetPath()), 
_sock(sock), _chunkedData(_router->GetChunkedData())
{
    _fileFD = -2;
    _totalSize = 0;
    _maxBodySize = _request.getMaxBodySize();
    _len = 0;
    if (_request.isChunkedTransferEncoding() == false)
        _contentLen = _request.getcontentlen();
    else
        _contentLen = SIZE_MAX;
    if (_request.getBody().length())
    {
        _buffer = (char *)(_request.getBody().c_str());
        _len = _request.getBody().length();
        _subBufferUploadStrategy = new SubBufferUploadStrategy();
        _createFile();
        _decode();
        if (_status < eComplete) {
            _buffer = NULL;
            return;
        }
        _uploadBuffer();
    }
    _buffer = Utility::GetBuffer();
}

UploadStrategy::~UploadStrategy()
{
    delete _subBufferUploadStrategy;
    if (_fileFD >= 0)
        close(_fileFD);
    if (_buffer)
        Utility::ReleaseBuffer(_buffer);
}

void UploadStrategy::_createFile()
{
    if (_path.isDirectory())
        _filename = _path.getFullPath() + "/" + Utility::getRandomStr();
    else
    {
        _filename = _path.getFullPath();
        _filename = Utility::addRandomStr(_filename);
    }
    _router->addFileUploaded(_filename);
    _fileFD = open(_filename.c_str(), O_WRONLY | O_CREAT, 0644);
    if (_fileFD < 0)
        _status = eOpenFileError;
}


void UploadStrategy::_uploadBuffer()
{
    if ((size_t)_len > _contentLen - _totalSize)
        _len = _contentLen - _totalSize;
    _totalSize += _len;
    _subBufferUploadStrategy->SetBuffer(_buffer, _len, _fileFD);
    int subStatus = _subBufferUploadStrategy->Execute();
    if (subStatus != AStrategy::eComplete)
        _status = eWriteError;
    else if (_totalSize == _contentLen)
        _status = eComplete;
}

void UploadStrategy::_decode()
{
    if (_request.isChunkedTransferEncoding()) {
        _chunkResult = _chunkedData.Feed(_buffer, _len);
        if (_chunkedData.GetStatus() == ChunkedData::eError)
        {
            DDEBUG("UploadStrategy") << "1-_decode(): ChunkedData error";
            _status = eChunkedError;
            return;
        }
        else if (_chunkedData.GetStatus() == ChunkedData::eComplete)
            _status = eComplete;
        _len = _chunkResult.decodedLen;
    }
    if (_totalSize > _maxBodySize)
        _status = eMaxBodySizeExceeded;
}

void UploadStrategy::_readData()
{
    size_t toRead = BUF_SIZE;
    if (_contentLen != SIZE_MAX && _contentLen - _totalSize < toRead)
        toRead = _contentLen - _totalSize;
    _len = read(_sock->GetFd(), _buffer, toRead);
    if (_len <= 0)
    {
        _status = eReadError;
        return;
    }
    _sock->UpdateTime();
    
    
    DDEBUG("UploadStrategy") << "UploadStrategy::_readData, "
        << "Read " << _len 
        << " bytes, totalSize=" << _totalSize 
        << ", contentLen=" << _contentLen 
        << ", maxSize=" << _maxBodySize;
}

int UploadStrategy::Execute()
{
    if (_status != eContinue)
        return _status;
    _readData();
    if (_status < eComplete)
        return _status;
    _decode();
    if (_status < eComplete)
        return _status;
    _uploadBuffer();
    return _status;
}