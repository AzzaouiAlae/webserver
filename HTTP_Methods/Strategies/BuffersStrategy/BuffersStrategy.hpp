#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"
class BuffersStrategy : public IStrategy
{
    vector<pair<char *, size_t> > &_buffers;
    size_t _currentBuffer;
    size_t _currentBufferOffset;
    SocketIO &_socketIO;
public:
    BuffersStrategy(vector<pair<char *, size_t> > &buffers, SocketIO &socketIO);
    ~BuffersStrategy();
    bool Execute();
};