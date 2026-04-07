#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"
#include "SubBufferUploadStrategy.hpp"

class UploadStrategy : public AStrategy
{
    Routing *_router;
    ClientRequest &_request;
    Path &_path;
    ClientSocket *_sock;
    string _filename;
    int _fileFD;
    size_t _totalSize;
    size_t _maxBodySize;
    int _len;
    char *_buffer;
    size_t _contentLen;
    SubBufferUploadStrategy *_subBufferUploadStrategy;
    ChunkedData::Result _chunkResult;
    ChunkedData &_chunkedData;
    
    void _createFile();
    void _readData();
    void _uploadBuffer();
    void _decode();
public:
    int Execute();
    UploadStrategy(Routing *router, ClientSocket *sock);
    ~UploadStrategy();
};

