#pragma once
#include "Headers.hpp"

class ChunkedData
{
    int _status;
    ssize_t _currentChunkSize;
    ssize_t _unchunkedSize;
    ssize_t _totalUnchunkedSize;
    ssize_t _startIdx;

    void _ParseChunkSize(string& data);
    void _decodeFirstChunk(string &data);
    void _checkHexString(const string &data);
    string _toHex(ssize_t num);
public:
    enum Status
    {
        eParssingChunkSize,
        eParsingChunk,
        eChunkComplete,
        eComplete,
        eError
    };
    ChunkedData();
    void SizeWritting(ssize_t size);
    int UnchunkData(string& data);
    
    ssize_t ChunkeData(string &data, ssize_t startIdx, ssize_t size);

    ssize_t GetCurrentChunkSize() const;
    ssize_t GetUnchunkedSize() const;
    ssize_t GetTotalUnchunkedSize() const;
    ssize_t GetStartIdx() const;
    int GetStatus() const;
};
