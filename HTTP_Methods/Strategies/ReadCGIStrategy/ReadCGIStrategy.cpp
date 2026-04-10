#include "ReadCGIStrategy.hpp"

ReadCGIStrategy::ReadCGIStrategy(Routing *router, ClientSocket *sock, char *buffer, int &len)
    : _router(router), _request(_router->GetRequest()), _sock(sock),
       _len(len), _totalSize(0), _chunkedData(router->GetChunkedData())
{
    _contentLen = _request.getcontentlen();
    _maxBodySize = _request.getMaxBodySize();
    if (_request.getBody().length())
    {
        _buffer = &_request.getBody()[0];
        _currentReadSize = _request.getBody().length();
        _decode();
        if (_currentReadSize > _len)
            _request.getBody().erase(_len);
        _buffer = NULL;
    }
    else if (!_request.isChunkedTransferEncoding() && _request.getcontentlen() == 0)
        _status = AStrategy::eComplete;
    _len = 0;
    _currentReadSize = 0;
    _buffer = buffer;
}

ReadCGIStrategy::~ReadCGIStrategy()
{}

void ReadCGIStrategy::_readFromSocket()
{
    size_t toRead = BUF_SIZE;
    if (!_request.isChunkedTransferEncoding()) 
    {
        if (_contentLen != SIZE_MAX && _contentLen - _totalSize < toRead)
            toRead = _contentLen - _totalSize;
    }
    if (toRead == 0) {
        _status = eComplete;
        return;
    }
    _currentReadSize = read(_sock->GetFd(), _buffer, toRead);
    if (_currentReadSize <= 0)
        _status = eReadError;
    _sock->UpdateTime();
}

void ReadCGIStrategy::_decode()
{
    if (_currentReadSize == 0)
    {
        if (_contentLen == _totalSize)
            _status = eComplete;
        return;
    }
    if (_request.isChunkedTransferEncoding())
    {
        _chunkResult = _chunkedData.Feed(_buffer, _currentReadSize);
        if (_chunkedData.GetStatus() == ChunkedData::eError)
        {
            DDEBUG("UploadStrategy") << "1-_decode(): ChunkedData error";
            _status = eChunkedError;
            return;
        }
        else if (_chunkedData.GetStatus() == ChunkedData::eComplete)
            _status = eComplete;
        _len += _chunkResult.decodedLen;
        _totalSize += _chunkResult.decodedLen;
    }
    else
    {
        _totalSize += _currentReadSize;
        _len += _currentReadSize;
        if (_contentLen == _totalSize)
            _status = eComplete;
    }
    if (_totalSize > _maxBodySize)
        _status = eMaxBodySizeExceeded;
}

int ReadCGIStrategy::Execute()
{
    if (_status != eContinue)
        return _status;
    if (BUF_SIZE - _len == 0)
        return _status;
    _readFromSocket();
    if (_status < eComplete)
        return _status;
    _decode();
    DDEBUG("UploadStrategy") << "Execute(): _len=" << _len
                            << ", _totalSize=" << _totalSize
                            << ", _status=" << _status;
    return _status;
}
