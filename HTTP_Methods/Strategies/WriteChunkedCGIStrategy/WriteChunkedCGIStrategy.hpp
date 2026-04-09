#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

#define CHUNK_HEADER_RESERVE  11
#define CHUNK_TRAILER_SIZE     2  

class WriteChunkedCGIStrategy : public AStrategy
{
    ClientSocket   *_sok;
    char       *_buffer;
    size_t        &_len;
    CgiRequest &_cgireq;
    bool       &_pipeEof;

    bool        _headerBuilt;
    string      _responseHeaderStr;
    size_t      _headerSended;

    bool        _hexSizeAdded;
    bool        _hasContentLength;
    size_t      _contentLength;
    size_t      _totalSent;

    size_t      _bodySended;
    size_t      _currentChunkSent;
    size_t      _terminatorSent;

    int         _totalFrame;
    char        *_sendStart;

    
    void _buildHeader();
    void _sendHeader();
    void _sendCgiRequestBody();
    void _streamDirect();
    void _sendChunked();
    void _sendTerminator();

public:
    WriteChunkedCGIStrategy(ClientSocket *sok, char *buffer, size_t &len,
                             CgiRequest &cgireq, bool &pipeEof);
    ~WriteChunkedCGIStrategy();
    int Execute();
};
