#include "MultipartData.hpp"
#include <algorithm>

void FormData::reset()
{
    name.clear();
    filename.clear();
    contentType.clear();
    bodyStart = 0;
    bodyLen = 0;
    isNewPart = false;
    bodyComplete = false;
}

MultipartData::MultipartData()
    : _status(eParsingBoundary), _parseIdx(0)
{
    _current.reset();
    _isInitialized = false;
}

void MultipartData::Initialize(const string &boundary)
{
    if (_isInitialized)
        return;
    _boundary = boundary;
    _delimiter = "\r\n--" + boundary;
    _isInitialized = true;
}

MultipartData::~MultipartData() {}

queue<FormData> &MultipartData::GetParts()
{
    return _parts;
}

MultipartData::Status MultipartData::GetStatus() const
{
    return _status;
}

void MultipartData::Feed(char *buf, size_t len)
{
    if (_status == eComplete || _status == eError)
        return;
    
    _parseIdx = 0;
    while (_parseIdx < len && _status != eComplete && _status != eError)
    {
        if (_status == eParsingBoundary)
            _parseBoundary(buf, len);
        else if (_status == eParsingHeader)
            _parseHeader(buf, len);
        else if (_status == eParsingBody)
            _scanBody(buf, len);
    }
}

void MultipartData::_parseBoundary(char *buf, size_t len)
{
    string target = "--" + _boundary + "\r\n";

    while (_parseIdx < len)
    {
        _partialMatch += buf[_parseIdx++];
        if (_partialMatch == target)
        {
            _partialMatch.clear();
            _status = eParsingHeader;
            _current.reset();
            _current.isNewPart = true;
            return;
        }
        if (target.find(_partialMatch) != 0)
        {
            _partialMatch.clear();
        }
    }
}

void MultipartData::_parseHeader(char *buf, size_t len) {
    while (_parseIdx < len) {
        char c = buf[_parseIdx++];
        _partialHeader += c;

        if (_partialHeader.size() == 2 && _partialHeader == "--") {
            _status = eComplete;
            _partialHeader.clear();
            return;
        }

        if (_partialHeader.size() >= 4 && 
            _partialHeader.substr(_partialHeader.size() - 4) == "\r\n\r\n") {
            
            size_t pos = 0;
            size_t next = 0;
            while ((next = _partialHeader.find("\r\n", pos)) != string::npos) {
                if (next > pos) {
                    _processHeaderLine(_partialHeader.substr(pos, next - pos));
                }
                pos = next + 2;
            }
            
            _partialHeader.clear();
            _status = eParsingBody;
            return;
        }
    }
}

void MultipartData::_processHeaderLine(const string &line)
{
    if (line.find("Content-Disposition:") == 0)
    {
        _parseContentDisposition(line);
    }
    else if (line.find("Content-Type:") == 0)
    {
        _parseContentType(line);
    }
}

string MultipartData::_extractValue(const string &line, const string &key)
{
    size_t pos = line.find(key + "=\"");
    if (pos == string::npos)
        return "";

    pos += key.length() + 2;
    size_t end = line.find("\"", pos);
    if (end == string::npos)
        return "";

    return line.substr(pos, end - pos);
}

void MultipartData::_parseContentDisposition(const string &line)
{
    _current.name = _extractValue(line, "name");
    _current.filename = _extractValue(line, "filename");
    _current.filename = Path::decodePath(_current.filename);
    Utility::normalizePath(_current.filename);
    Utility::addRandomStr(_current.filename);
    size_t pos = 0;
    while ((pos = _current.filename.find("/", pos)) != string::npos) {
        _current.filename[pos] = '_';
        pos += 1;
    }
}

void MultipartData::_parseContentType(const string &line)
{
    size_t pos = line.find(":");
    if (pos != string::npos && pos + 1 < line.length())
    {
        size_t start = line.find_first_not_of(" ", pos + 1);
        if (start != string::npos)
        {
            _current.contentType = line.substr(start);
        }
    }
}

void MultipartData::_emitBodyPart(size_t start, size_t len, bool complete)
{
    if (len > 0 || complete)
    {
        _current.bodyStart = start;
        _current.bodyLen = len;
        _current.bodyComplete = complete;
        _parts.push(_current);

        _current.isNewPart = false;
    }
}

size_t MultipartData::_findPartialDelimiter(char *buf, size_t len)
{
    size_t startCheck = (len > _delimiter.length()) ? (len - _delimiter.length()) : 0;

    for (size_t i = startCheck; i < len; ++i)
    {
        if (equal(buf + i, buf + len, _delimiter.begin()))
        {
            return i;
        }
    }
    return string::npos;
}

bool MultipartData::_processPartialMatch(char *buf, size_t len) {
    size_t avail = len - _parseIdx;
    size_t takeLen = std::min(avail, _delimiter.length());
    string window = _partialMatch + string(buf + _parseIdx, takeLen);
    
    size_t pos = window.find(_delimiter);
    if (pos != string::npos) {
        size_t boundaryEnd = pos + _delimiter.length();
        size_t consumedFromBuf = boundaryEnd - _partialMatch.length();
        
        _parseIdx += consumedFromBuf;
        _emitBodyPart(0, 0, true);
        _partialMatch.clear();
        
        _status = eParsingHeader;
        _current.reset();
        _current.isNewPart = true;
        return true;
    }

    size_t needed = _delimiter.length() - _partialMatch.length();
    size_t compareLen = std::min(needed, avail);
    
    if (std::equal(buf + _parseIdx, buf + _parseIdx + compareLen, _delimiter.begin() + _partialMatch.length())) {
        _partialMatch.append(buf + _parseIdx, compareLen);
        _parseIdx += compareLen;
        return true; 
    }
    
    return false;
}

void MultipartData::_searchForBoundary(char *buf, size_t searchStart, size_t len) {
    char *it = std::search(buf + searchStart, buf + len, _delimiter.begin(), _delimiter.end());

    if (it != buf + len) {
        size_t matchIdx = it - buf;
        _emitBodyPart(searchStart, matchIdx - searchStart, true);

        _parseIdx = matchIdx + _delimiter.length();
        _status = eParsingHeader;
        _current.reset();
        _current.isNewPart = true;
    } else {
        size_t partialIdx = _findPartialDelimiter(buf + searchStart, len - searchStart);

        if (partialIdx != string::npos) {
            partialIdx += searchStart;
            _emitBodyPart(searchStart, partialIdx - searchStart, false);
            _partialMatch.assign(buf + partialIdx, len - partialIdx);
            _parseIdx = len;
        } else {
            _emitBodyPart(searchStart, len - searchStart, false);
            _parseIdx = len;
        }
    }
}

void MultipartData::_scanBody(char *buf, size_t len) {
    size_t searchStart = _parseIdx;

    if (!_partialMatch.empty()) {
        if (_processPartialMatch(buf, len)) {
            return;
        }
        _partialMatch.clear();
        searchStart = _parseIdx;
    }

    _searchForBoundary(buf, searchStart, len);
}