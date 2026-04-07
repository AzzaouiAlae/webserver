#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

class WriteToPipeStrategy : public AStrategy
{
    int _outputFd;
    ClientRequest &_request;
    char *_buffer;
    int &_len;
    int _writtenBytes;
    size_t _totalToWrite;
    int _offset;
    
    void _writeToPipe();
public:
    WriteToPipeStrategy(Routing *routing, int outputFd, char *buffer, int &len);
    ~WriteToPipeStrategy();
    int Execute();
};