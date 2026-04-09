#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"

#define CHUNK_HEADER_RESERVE  11
#define CHUNK_TRAILER_SIZE     2

class ReadPipeStrategy : public AStrategy
{
    int         _pipeFd;
    char       *_buffer;
    size_t       &_len;
    CgiRequest &_cgireq;
    bool       &_headerParsed;
    bool        _chunkedMode;
                                
    size_t      _totalRead; 


    void _readHeader();
    void _readBody();

public:
    ReadPipeStrategy(int pipeFd, char *buffer, size_t &len,
                     CgiRequest &cgireq, bool &headerParsed, bool chunkedMode);
    ~ReadPipeStrategy();
    int Execute();
};
