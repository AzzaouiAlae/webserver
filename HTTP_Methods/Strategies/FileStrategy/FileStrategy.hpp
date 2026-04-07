#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

class FileStrategy : public AStrategy
{
    int _fileFd;
    ClientSocket &_socketIO;
    AStrategy *_buffersStrategy;
    size_t _fileSize;
    size_t _sentFileSize;
    bool _isBufferSent;
public:
    FileStrategy(vector<pair<char *, size_t> > &buffers, int fileFd, size_t fileSize, ClientSocket &socketIO);
    ~FileStrategy();
    int Execute();
};
