#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

class ReadCGIStrategy : public AStrategy
{
    Routing *_router;
    ClientRequest &_request;
    ClientSocket *_sock;
    char *_buffer;
    int &_len;
    int _currentReadSize;
    size_t _totalSize;
    size_t _contentLen;
    size_t _maxBodySize;
    ChunkedData &_chunkedData;
    ChunkedData::Result _chunkResult;

    void _readFromSocket();
    void _decode();
public:
    ReadCGIStrategy(Routing *router, ClientSocket *sock, char *buffer, int &len);
    ~ReadCGIStrategy();
    int Execute();
};