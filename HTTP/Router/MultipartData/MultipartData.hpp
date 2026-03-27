#pragma once
#include "Headers.hpp"

#define MAX_HEADER_SIZE 4096

struct FormData {
private: 
    string *_rowData;
    int fd;
public:
    FormData(string &data);
    ~FormData();

    string name;
    string filename;
    string contentType;
    char *data();
    ssize_t startIdx;
    ssize_t size;
    bool isComplete;
    bool isHeaderParsed;
    
    int openFile();
};

class MultipartData {
    
    size_t _startHeaderIdx;
    size_t _idx;
    int _status;
    bool _isIncompleteParse;
    bool _contentDispositionParsed;
    bool _contentTypeParsed;
    queue<FormData *> _formData;
    FormData *_currentFormData;
    void _parseBoundry(string &data, const string &boundary);
    void _parseContentDisposition(string &data);
    void _parseContentType(string &data);
    void _parseHeader(string &data);
    void _parseBody(string &data, const string &boundary);
    void _parseBodyComplete();
public:
    enum Status {
        eParseBoundary,
        eParseHeader,
        eParseBody,
        eParseBodyComplete,
        eComplete,
        eError
    };
    MultipartData();
    ~MultipartData();
    int Parse(string &data, const string &boundary);
    queue<FormData *> &getFormData();
    FormData *getCurrentFormData();
    size_t getLastIndex();
    void resetIndex();
};
