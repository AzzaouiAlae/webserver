#pragma once
#include "Headers.hpp"

class ChunkedData {
public:
    enum Status { eParsingSize, eParsingBody, eComplete, eError };

    struct Result {
        size_t rawConsumed;  // how many bytes of buf were processed
        size_t decodedLen;   // decoded bytes now at buf[0 .. decodedLen)
    };

    ChunkedData();

    // Decodes buf[0..len) in-place. Decoded bytes land at buf[0..Result.decodedLen).
    // rawConsumed tells the caller how many raw bytes to discard before the next read.
    Result Feed(char *buf, size_t len);

    Status GetStatus() const;

private:
    Status  _status;
    ssize_t _remaining;    // body bytes still expected in the current chunk
    string  _partialSize;  // leftover hex digits when a size line is split across reads

    void _parseSize(char *buf, size_t &r, size_t end);
    void _copyBody(char *buf, size_t &r, size_t &w, size_t end);
};