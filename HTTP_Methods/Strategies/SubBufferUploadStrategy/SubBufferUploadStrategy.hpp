#pragma once
#include "Headers.hpp"

class SubBufferUploadStrategy: public AStrategy
{
    char *_buffer;
    size_t _bufferSize;
    int _uploadFd;
public:
    SubBufferUploadStrategy();
    void SetBuffer(char *buffer, size_t bufferSize, int uploadFd);
    virtual ~SubBufferUploadStrategy();
    virtual int Execute();
};