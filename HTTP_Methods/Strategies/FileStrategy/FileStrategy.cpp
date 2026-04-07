#include "FileStrategy.hpp"
#include "BuffersStrategy.hpp"

FileStrategy::FileStrategy(vector<pair<char *, size_t> > &buffers, int fileFd, size_t fileSize, ClientSocket &socketIO)
    : _fileFd(fileFd), _socketIO(socketIO), _fileSize(fileSize), _sentFileSize(0), _isBufferSent(false)
{
    _buffersStrategy = new BuffersStrategy(buffers, _socketIO);
}

FileStrategy::~FileStrategy()
{
    delete _buffersStrategy;
}

int FileStrategy::Execute()
{
    int status;
    if (_isBufferSent)
    {
        ssize_t sent = NetIO::FileToSocket(_fileFd, _fileSize - _sentFileSize, _socketIO.GetFd());
        if (sent <= 0)
        {
            _socketIO.SetCloseConnection(true);
            _status = eWriteError;
        }
        else 
        {
            _sentFileSize += sent;
            if (_sentFileSize >= _fileSize)
                _status = eComplete;
            _socketIO.UpdateTime();
        }
    }
    else
    {
        status = _buffersStrategy->Execute();
        if (status == AStrategy::eWriteError)
        {
            _socketIO.SetCloseConnection(true);
            _status = eWriteError;
        }
        else if (status == AStrategy::eComplete)
            _isBufferSent = true;
        if (status == AStrategy::eComplete && _fileSize == 0)
            _status = eComplete;
    }
    return _status;
}
