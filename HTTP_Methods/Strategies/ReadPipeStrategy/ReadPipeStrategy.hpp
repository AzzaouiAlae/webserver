#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

#define CHUNK_HEADER_RESERVE  20  // max hex size line: "FFFFFFFFFFFFFFFF\r\n" = 18 + null
#define CHUNK_TRAILER_SIZE     2  // \r\n after chunk data

class ReadPipeStrategy : public AStrategy
{
    int         _pipeFd;        // CGI stdout read end (_sok->pipefd[0])
    char       *_buffer;        // shared _readBuffer with write strategy
    size_t       &_len;           // _availableCgiBufferLen
    CgiRequest &_cgireq;        // stateful CGI header parser (accumulates internally)
    bool       &_headerParsed;  // set by Cgi after _createCgiResponse()
    bool        _chunkedMode;   // true  → read at _buffer + CHUNK_HEADER_RESERVE (zero-copy chunked)
                                // false → read at _buffer + _len (normal accumulation)
    size_t      _totalRead;     // accounting/debug only

    // Phase 1: feed raw bytes to _cgireq until headers complete
    // Always reads at _buffer + 0 (scratch, _cgireq owns the data internally)
    // Returns: sets _status to eReadComplete when _cgireq.isComplete() → true
    //          sets _status to eComplete   on EOF (Cgi must 502 if !_headerParsed)
    //          sets _status to eReadError  on read() < 0
    void _readHeader();

    // Phase 2: fill buffer for write strategy
    // chunkedMode == true:
    //   - if _len > 0: write strategy hasn't sent current chunk yet → return (backpressure)
    //   - reads into _buffer + CHUNK_HEADER_RESERVE, sets _len = bytes read
    // chunkedMode == false:
    //   - if BUF_SIZE - _len == 0: buffer full, wait → return
    //   - reads into _buffer + _len, increments _len
    void _readBody();

public:
    ReadPipeStrategy(int pipeFd, char *buffer, size_t &len,
                     CgiRequest &cgireq, bool &headerParsed, bool chunkedMode);
    ~ReadPipeStrategy();
    int Execute();

    // Status contract:
    // eContinue    → keep calling Execute() on next EPOLLIN
    // eReadComplete→ header fully parsed this call; Cgi must call _createCgiResponse()
    //                then call Execute() again to start body reading
    // eComplete    → EOF on pipe; if !_headerParsed this means 502 in Cgi
    // eReadError   → read() returned < 0 → Cgi sets 502
};
