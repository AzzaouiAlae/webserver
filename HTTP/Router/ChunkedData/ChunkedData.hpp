#pragma once
#include "Headers.hpp"

class ChunkedData {
public:
    enum Status { eParsingSize, eParsingBody, eComplete, eError };

    struct Result {
        size_t rawConsumed;
        size_t decodedLen;
    };

    ChunkedData();
    Result Feed(char *buf, size_t len);
    Status GetStatus() const;

private:
    Status  _status;
    ssize_t _remaining;
    string  _partialSize;

    void _parseSize(char *buf, size_t &r, size_t end);
    void _copyBody(char *buf, size_t &r, size_t &w, size_t end);
};