#include "FileStrategy.hpp"
#include "BuffersStrategy.hpp"

FileStrategy::FileStrategy(vector<pair<char *, size_t> > &buffers, int fileFd, size_t fileSize, SocketIO &socketIO)
    : _fileFd(fileFd), _socketIO(socketIO), _fileSize(fileSize), _sentFileSize(0), _isBufferSent(false)
{
    _buffersStrategy = new BuffersStrategy(buffers, _socketIO);
}

FileStrategy::~FileStrategy()
{
    if (_fileFd != -1)
        close(_fileFd);
    delete _buffersStrategy;
}

int FileStrategy::Execute()
{
    int status;
    if (_isBufferSent)
    {
        ssize_t sent = _socketIO.FileToSocket(_fileFd, _fileSize - _sentFileSize);
        if (sent <= 0)
        {
            _socketIO.closeConnection = true;
            _status = eWriteError;
        }
        else 
        {
            _sentFileSize += sent;
            if (_sentFileSize >= _fileSize)
                _status = eComplete;
        }
    }
    else
    {
        status = _buffersStrategy->Execute();
        if (status == AStrategy::eWriteError)
        {
            _socketIO.closeConnection = true;
            _status = eWriteError;
        }
        else if (status == AStrategy::eComplete)
            _isBufferSent = true;
    }
    return _status;
}
