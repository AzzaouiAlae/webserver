#include "BuffersStrategy.hpp"

BuffersStrategy::BuffersStrategy(vector<pair<char *, size_t> > &buffers, ClientSocket &socketIO) : 
AStrategy(), _buffers(buffers), _currentBuffer(0), _currentBufferOffset(0), _socketIO(socketIO)
{}

BuffersStrategy::~BuffersStrategy()
{}

int BuffersStrategy::Execute()
{
    DDEBUG("BuffersStrategy") 
        << "Executing BuffersStrategy:" 
        << "currentBuffer=" << _currentBuffer 
        << ", currentBufferOffset=" << _currentBufferOffset 
        << ",  status=" << _status
        << ", buffers size=" << _buffers.size();
       
    if (_status != eContinue)
        return _status;
    pair<char *, size_t> &current = _buffers[_currentBuffer];
    int sent = NetIO::Send(current.first + _currentBufferOffset, current.second - _currentBufferOffset, _socketIO.GetFd());
    if (sent <= 0) {
        _socketIO.SetCloseConnection(true);
        _status = eWriteError;
        return _status;
    }
    else
        _socketIO.SetSendStart(true);
    DDEBUG("BuffersStrategy") 
        << "Executing BuffersStrategy:"
        << ", str sended\n"
        << string(current.first + _currentBufferOffset, sent);
    _currentBufferOffset += sent;
    if (_currentBufferOffset >= current.second) 
    {
        _currentBuffer++;
        _currentBufferOffset = 0;
    }
    if (_currentBuffer >= _buffers.size())
        _status = eComplete;
    _socketIO.UpdateTime();
    
    return _status;
}
