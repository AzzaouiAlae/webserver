#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"
class BuffersStrategy : public AStrategy
{
    vector<pair<char *, size_t> > &_buffers;
    size_t _currentBuffer;
    size_t _currentBufferOffset;
    ClientSocket &_socketIO;
public:
    BuffersStrategy(vector<pair<char *, size_t> > &buffers, ClientSocket &socketIO);
    ~BuffersStrategy();
    int Execute();
};