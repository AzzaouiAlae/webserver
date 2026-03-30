#include "BuffersStrategy.hpp"

BuffersStrategy::BuffersStrategy(vector<pair<char *, size_t> > &buffers, SocketIO &socketIO) : AStrategy(), _buffers(buffers), _currentBuffer(0), _currentBufferOffset(0), _socketIO(socketIO)
{}

BuffersStrategy::~BuffersStrategy()
{}

int BuffersStrategy::Execute()
{
    DEBUG("BuffersStrategy") 
        << "Executing BuffersStrategy:" 
        << "currentBuffer=" << _currentBuffer 
        << ", currentBufferOffset=" << _currentBufferOffset 
        << ",  status=" << _status;
    if (_status != eContinue)
        return _status;
    pair<char *, size_t> &current = _buffers[_currentBuffer];
    int sent = _socketIO.Send(current.first + _currentBufferOffset, current.second - _currentBufferOffset);
    if (sent <= 0) {
        _socketIO.closeConnection = true;
        _status = eWriteError;
        return _status;
    }
    _currentBufferOffset += sent;
    if (_currentBufferOffset >= current.second) 
    {
        _currentBuffer++;
        _currentBufferOffset = 0;
    }
    if (_currentBuffer >= _buffers.size())
        _status = eComplete;
    return _status;
}
