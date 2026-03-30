#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"
class BuffersStrategy : public AStrategy
{
    vector<pair<char *, size_t> > &_buffers;
    size_t _currentBuffer;
    size_t _currentBufferOffset;
    SocketIO &_socketIO;
public:
    BuffersStrategy(vector<pair<char *, size_t> > &buffers, SocketIO &socketIO);
    ~BuffersStrategy();
    int Execute();
};