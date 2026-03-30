#pragma once
#include "Headers.hpp"
#include "SocketIO.hpp"
#include "SubBufferUploadStrategy.hpp"
#include "Path.hpp"

class MulipartUploadStrategy: public AStrategy
{
    char *_buffer;
    SocketIO *_socketIO;
    SubBufferUploadStrategy *_subBufferUploadStrategy;
    Routing *_routing;
    ClientRequest &_request;
    ChunkedData &_chunkedData;
    MultipartData &_multipartData;
    string &_str;
    string _boundery;
    int _chunkStatus, _multipartStatus;
    void _uploadBuffers(queue<FormData *> &formDataQueue);
    void _decodeData();
    void _uploadBuffer(FormData *formData);
public:
    MulipartUploadStrategy(SocketIO *socketIO, Routing *routing);
    virtual ~MulipartUploadStrategy();
    virtual int Execute();
};