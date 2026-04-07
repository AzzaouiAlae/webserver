#pragma once
#include "Headers.hpp"

struct FormData {
    string name;
    string filename;
    string contentType;
    
    size_t bodyStart;
    size_t bodyLen;
    
    bool   isNewPart;
    bool   bodyComplete;
    
    void reset();
};

class MultipartData {
public:
    enum Status { eParsingBoundary, eParsingHeader, eParsingBody, eComplete, eError };

    MultipartData();
    ~MultipartData();

    void Feed(char *buf, size_t len);
    queue<FormData> &GetParts();
    Status GetStatus() const;
    void Initialize(const string &boundary);

private:
    string           _boundary;
    string           _delimiter;
    string           _partialMatch;
    Status           _status;
    FormData         _current;
    size_t           _parseIdx;
    string           _partialHeader;
    queue<FormData> _parts;
    bool _isInitialized;

    void _parseBoundary(char *buf, size_t len);
    void _parseHeader(char *buf, size_t len);
    void _processHeaderLine(const string &line);
    void _parseContentDisposition(const string &line);
    void _parseContentType(const string &line);
    void _scanBody(char *buf, size_t len);
    void _emitBodyPart(size_t start, size_t len, bool complete);
    size_t _findPartialDelimiter(char *buf, size_t len);
    string _extractValue(const string &line, const string &key);
    bool _processPartialMatch(char *buf, size_t len);
    void _searchForBoundary(char *buf, size_t searchStart, size_t len);
};