#pragma once
#include "Headers.hpp"
#include "NetIO.hpp"
#include "FileStrategy.hpp"
#include "BuffersStrategy.hpp"

#define MAX_MEM_BUFFER (5 * 1024 * 1024)

class WriteBufferedCGIStrategy : public AStrategy
{
private:
    enum InternalState {
        eInit,
        eSendingDirectHeader,
        eStreamingDirect,
        eBuffering,
        eSendingDelegate
    };

    ClientSocket   *_sok;
    char       *_buffer;
    size_t     &_len;
    CgiRequest &_cgireq;
    bool       &_pipeEof;

    InternalState _internalState;
    bool _hasContentLength;
    
    // --- Buffering State ---
    std::vector<char> _memBuffer; 
    int _tempFd;                  
    size_t _fileSize;
    
    // --- Delegation State ---
    string _responseHeaderStr;
    size_t _headerSent;
    size_t _bodySended;
    
    // Holds the pair of pointers for Headers and Memory Buffer
    vector<pair<char *, size_t> > _sendBuffers; 
    AStrategy *_delegate;
    
    // --- Internal Methods ---
    void _createTempFile();
    void _accumulate();
    void _streamDirect();
    void _setupDelegate();
    void _buildHeader(size_t contentLength = 0);

public:
    WriteBufferedCGIStrategy(ClientSocket *sok, char *buffer, size_t &len, 
                             CgiRequest &cgireq, bool &pipeEof);
    ~WriteBufferedCGIStrategy();
    
    int Execute();
};