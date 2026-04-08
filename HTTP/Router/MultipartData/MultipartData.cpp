#include "MultipartData.hpp"
#include <algorithm>

// ═════════════════════════════════════════════════════════════════════════════
// FormData
// ═════════════════════════════════════════════════════════════════════════════

void FormData::reset()
{
    name.clear();
    filename.clear();
    contentType.clear();
    bodyPart.clear();
    bodyStart = 0;
    bodyLen = 0;
    isNewPart = false;
    bodyComplete = false;
}

// ═════════════════════════════════════════════════════════════════════════════
// Construction / public interface
// ═════════════════════════════════════════════════════════════════════════════

MultipartData::MultipartData()
    : _status(eParsingBoundary), _isInitialized(false), _parseIdx(0)
{
    _current.reset();
}

MultipartData::~MultipartData() {}

void MultipartData::Initialize(const string &boundary)
{
    if (_isInitialized)
        return;
    _boundary = boundary;
    _delimiter = "\r\n--" + boundary;
    _isInitialized = true;
}

queue<FormData> &MultipartData::GetParts() { return _parts; }
MultipartData::Status MultipartData::GetStatus() const { return _status; }

// ─────────────────────────────────────────────────────────────────────────────
// Feed  –  dispatch loop
// ─────────────────────────────────────────────────────────────────────────────

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

// ═════════════════════════════════════════════════════════════════════════════
// Opening boundary  (first "--boundary\r\n" before any header)
// ═════════════════════════════════════════════════════════════════════════════

void MultipartData::_parseBoundary(char *buf, size_t len)
{
    string target = "--" + _boundary + "\r\n";

    while (_parseIdx < len)
    {
        _partialMatch += buf[_parseIdx++];

        if (_partialMatch == target)
        {
            _partialMatch.clear();
            _current.reset();
            _current.isNewPart = true;
            _status = eParsingHeader;
            return;
        }

        if (target.find(_partialMatch) != 0)
            _partialMatch.clear();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Header block
// ═════════════════════════════════════════════════════════════════════════════

// Accumulate bytes until the blank line ("\r\n\r\n") that ends the headers.
void MultipartData::_parseHeader(char *buf, size_t len)
{
    while (_parseIdx < len)
    {
        _partialHeader += buf[_parseIdx++];

        if (_partialHeader.size() >= 4 &&
            _partialHeader.substr(_partialHeader.size() - 4) == "\r\n\r\n")
        {
            _dispatchHeaderLines(_partialHeader);
            _partialHeader.clear();
            _status = eParsingBody;
            return;
        }
    }
}

// Split the accumulated header block on "\r\n" and process each line.
void MultipartData::_dispatchHeaderLines(const string &block)
{
    size_t pos = 0;
    size_t next;
    while ((next = block.find("\r\n", pos)) != string::npos)
    {
        if (next > pos)
            _processHeaderLine(block.substr(pos, next - pos));
        pos = next + 2;
    }
}

void MultipartData::_processHeaderLine(const string &line)
{
    if (line.find("Content-Disposition:") == 0)
        _parseContentDisposition(line);
    else if (line.find("Content-Type:") == 0)
        _parseContentType(line);
}

// Extract the value of a quoted attribute: key="value"
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
    _current.filename = Utility::addRandomStr(_current.filename);

    size_t pos = 0;
    while ((pos = _current.filename.find("/", pos)) != string::npos)
        _current.filename[pos++] = '_';
}

void MultipartData::_parseContentType(const string &line)
{
    size_t colon = line.find(":");
    if (colon == string::npos || colon + 1 >= line.length())
        return;
    size_t start = line.find_first_not_of(" ", colon + 1);
    if (start != string::npos)
        _current.contentType = line.substr(start);
}

// ═════════════════════════════════════════════════════════════════════════════
// Body scanning helpers
// ═════════════════════════════════════════════════════════════════════════════

// A boundary is valid only when followed by "\r\n" (next part) or "--" (close).
bool MultipartData::_isValidSuffix(const char *suffix, bool &isClosing) const
{
    isClosing = (suffix[0] == '-' && suffix[1] == '-');
    return isClosing || (suffix[0] == '\r' && suffix[1] == '\n');
}

// Push the current body chunk to the queue.
// bodyPart (leftover partial bytes from the previous chunk) is emitted first,
// then the slice buf[start .. start+len) is described by bodyStart/bodyLen.
void MultipartData::_emitBodyChunk(size_t start, size_t len, bool complete)
{
    if (!_current.bodyPart.empty() || len > 0 || complete)
    {
        _current.bodyStart = start;
        _current.bodyLen = len;
        _current.bodyComplete = complete;
        _parts.push(_current);
        _current.bodyPart.clear();
        _current.isNewPart = false;
    }
}

// Set the next state after a confirmed boundary.
void MultipartData::_transitionAfterBoundary(bool isClosing)
{
    _partialMatch.clear();
    if (isClosing)
        _status = eComplete;
    else
    {
        _current.reset();
        _current.isNewPart = true;
        _status = eParsingHeader;
    }
}

// Check whether the delimiter begins to appear at the tail of buf[0..len).
// (A partial match that needs more data to be confirmed.)
// Returns the start index of the partial match, or string::npos if none.
size_t MultipartData::_findTailPartial(char *buf, size_t len) const
{
    size_t startCheck = (len > _delimiter.length()) ? len - _delimiter.length() : 0;

    for (size_t i = startCheck; i < len; ++i)
    {
        if (std::equal(buf + i, buf + len, _delimiter.begin()))
            return i;
    }
    return string::npos;
}

// ═════════════════════════════════════════════════════════════════════════════
// Body scanning
// ═════════════════════════════════════════════════════════════════════════════

// Resolve _partialMatch that was saved at the end of the previous chunk.
//
// Returns true  – partial was handled (boundary found or still accumulating).
//                 Caller should return immediately.
// Returns false – partial turned out to be plain body bytes.
//                 They are stored in _current.bodyPart; caller continues scanning.
bool MultipartData::_resolvePartialMatch(char *buf, size_t len)
{
    size_t avail = len - _parseIdx;
    size_t needed = _delimiter.length() + 2 - _partialMatch.length();
    size_t takeLen = std::min(avail, needed);
    string window = _partialMatch + string(buf + _parseIdx, takeLen);

    size_t pos = window.find(_delimiter);

    // Delimiter found but suffix bytes not yet available — keep accumulating.
    if (pos != string::npos && window.length() < _delimiter.length() + 2)
    {
        _partialMatch.append(buf + _parseIdx, takeLen);
        _parseIdx += takeLen;
        return true;
    }

    // Delimiter found and we have the 2 suffix bytes to validate it.
    if (pos != string::npos)
    {
        bool isClosing = false;
        if (!_isValidSuffix(window.c_str() + _delimiter.length(), isClosing))
        {
            // False positive — the partial bytes are plain body content.
            _current.bodyPart = _partialMatch;
            _partialMatch.clear();
            return false;
        }
        _parseIdx += takeLen;
        _emitBodyChunk(0, 0, true);
        _transitionAfterBoundary(isClosing);
        return true;
    }

    // Delimiter not in window — check if the buffer still extends the partial.
    if (std::equal(buf + _parseIdx, buf + _parseIdx + takeLen,
                   _delimiter.begin() + _partialMatch.length()))
    {
        _partialMatch.append(buf + _parseIdx, takeLen);
        _parseIdx += takeLen;
        return true;
    }

    // No match at all — treat partial as body.
    _current.bodyPart = _partialMatch;
    _partialMatch.clear();
    return false;
}

// Scan buf[searchStart..len) for a complete, validated boundary.
//
// False positives (delimiter string without a valid suffix) are skipped:
// emitStart is kept in place so those bytes are included in the next emit,
// and scanning resumes past the false match.
void MultipartData::_scanForBoundary(char *buf, size_t searchStart, size_t len)
{
    size_t emitStart = searchStart;
    size_t scanPos = searchStart;

    while (true)
    {
        char *it = std::search(buf + scanPos, buf + len,
                               _delimiter.begin(), _delimiter.end());

        // ── No delimiter in the remaining buffer ─────────────────────────────
        if (it == buf + len)
        {
            size_t partialIdx = _findTailPartial(buf + emitStart, len - emitStart);
            if (partialIdx != string::npos)
            {
                partialIdx += emitStart;
                _emitBodyChunk(emitStart, partialIdx - emitStart, false);
                _partialMatch.assign(buf + partialIdx, len - partialIdx);
            }
            else
                _emitBodyChunk(emitStart, len - emitStart, false);
            _parseIdx = len;
            return;
        }

        size_t matchIdx = it - buf;
        size_t suffixOff = matchIdx + _delimiter.length();

        // ── Delimiter found but suffix straddles the chunk end ───────────────
        if (suffixOff + 2 > len)
        {
            _emitBodyChunk(emitStart, matchIdx - emitStart, false);
            _partialMatch.assign(buf + matchIdx, len - matchIdx);
            _parseIdx = len;
            return;
        }

        // ── Validate the 2 bytes that follow the delimiter ───────────────────
        bool isClosing = false;
        if (!_isValidSuffix(buf + suffixOff, isClosing))
        {
            scanPos = suffixOff;
            continue;
        }

        // ── Real boundary confirmed ──────────────────────────────────────────
        _emitBodyChunk(emitStart, matchIdx - emitStart, true);
        _parseIdx = suffixOff + 2;
        _transitionAfterBoundary(isClosing);
        return;
    }
}

void MultipartData::_scanBody(char *buf, size_t len)
{
    if (!_partialMatch.empty() && _resolvePartialMatch(buf, len))
        return;

    // If _resolvePartialMatch returned false, the discarded partial bytes are
    // already stored in _current.bodyPart and will be emitted by _emitBodyChunk.
    _scanForBoundary(buf, _parseIdx, len);
}