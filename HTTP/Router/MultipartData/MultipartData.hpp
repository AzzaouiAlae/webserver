#pragma once
#include "Headers.hpp"

struct FormData {
    string name;
    string filename;
    string contentType;
    string bodyPart;

    size_t bodyStart;
    size_t bodyLen;

    bool isNewPart;
    bool bodyComplete;

    void reset();
};

class MultipartData {
public:
    enum Status { eParsingBoundary, eParsingHeader, eParsingBody, eComplete, eError };

    MultipartData();
    ~MultipartData();

    void            Initialize(const string &boundary);
    void            Feed(char *buf, size_t len);
    queue<FormData> &GetParts();
    Status          GetStatus() const;

private:
    // ── state ────────────────────────────────────────────────────────────────
    Status          _status;
    bool            _isInitialized;
    string          _boundary;
    string          _delimiter;
    FormData        _current;
    queue<FormData> _parts;

    // ── per-chunk working variables ──────────────────────────────────────────
    size_t          _parseIdx;
    string          _partialMatch;
    string          _partialHeader; 

    // ── opening boundary ─────────────────────────────────────────────────────
    void   _parseBoundary(char *buf, size_t len);

    // ── header block ─────────────────────────────────────────────────────────
    void   _parseHeader(char *buf, size_t len);
    void   _dispatchHeaderLines(const string &block);
    void   _processHeaderLine(const string &line);
    void   _parseContentDisposition(const string &line);
    void   _parseContentType(const string &line);
    string _extractValue(const string &line, const string &key);

    // ── body scanning ────────────────────────────────────────────────────────
    void   _scanBody(char *buf, size_t len);
    bool   _resolvePartialMatch(char *buf, size_t len);
    void   _scanForBoundary(char *buf, size_t searchStart, size_t len);

    // ── helpers ──────────────────────────────────────────────────────────────
    bool   _isValidSuffix(const char *suffix, bool &isClosing) const;
    void   _emitBodyChunk(size_t start, size_t len, bool complete);
    void   _transitionAfterBoundary(bool isClosing);
    size_t _findTailPartial(char *buf, size_t len) const;
};