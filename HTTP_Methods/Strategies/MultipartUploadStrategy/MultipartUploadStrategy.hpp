#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"
#include "SubBufferUploadStrategy.hpp"
#include "Path.hpp"

class MultipartUploadStrategy: public AStrategy
{
    char *_buffer;
    ClientSocket *_socketIO;
    SubBufferUploadStrategy *_subBufferUploadStrategy;
    Routing *_router;
    ClientRequest &_request;
    ChunkedData &_chunkedData;
    MultipartData &_multipartData;
    ChunkedData::Result _chunkResult;
    ssize_t _contentLen;
    size_t _totalSize;
    size_t _maxSize;
    int _len;
    void _uploadBuffers(queue<FormData> &formDataQueue);
    void _decodeData();
    void _uploadBuffer(FormData *formData);
    bool _needToRead();
    void _updateStatus();
public:
    MultipartUploadStrategy(ClientSocket *socketIO, Routing *routing);
    virtual ~MultipartUploadStrategy();
    virtual int Execute();
};