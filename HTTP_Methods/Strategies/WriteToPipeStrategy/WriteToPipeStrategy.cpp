#include "WriteToPipeStrategy.hpp"

WriteToPipeStrategy::WriteToPipeStrategy(Routing *routing, int outputFd, char *buffer, int &len)
    : _outputFd(outputFd), _request(routing->GetRequest()), _buffer(buffer), _len(len)
{
    _writtenBytes = 0;
    _totalToWrite = 0;
    _offset = 0;
}

WriteToPipeStrategy::~WriteToPipeStrategy()
{}

void WriteToPipeStrategy::_writeToPipe()
{
    if (_request.getBody().length() > 0)
    {
        char *data = &_request.getBody()[_writtenBytes];
        int len = _request.getBody().length() - _writtenBytes;
        int sent = write(_outputFd, data, len);
        if (sent <= 0)
            _status = eWriteError;
        else {
            _writtenBytes += sent;
            _totalToWrite += sent;
        }
        if (_writtenBytes >= (int)_request.getBody().length())
        {
            _request.getBody().clear();
            _writtenBytes = 0;
        }
    }
    else 
    {
        if (_len <= 0)
            return;
        int sent = write(_outputFd, _buffer + _offset, _len);
        if (sent <= 0)
            _status = eWriteError;
        else
        {
            _len -= sent;
            _offset += sent;
            _totalToWrite += sent;
            if (_len == 0)
                _offset = 0;
        }
    }
    DEBUG("WriteToPipeStrategy") << "_writeToPipe(): total written to pipe: " << _totalToWrite;
}

int WriteToPipeStrategy::Execute()
{
    if (_status != eContinue)
        return _status;
    _writeToPipe();
    DDEBUG("WriteToPipeStrategy") << "Execute(): total written to pipe: " << _totalToWrite;
    return _status;
}