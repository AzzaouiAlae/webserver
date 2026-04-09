#include "WriteChunkedCGIStrategy.hpp"
#include "AMethod.hpp"

WriteChunkedCGIStrategy::WriteChunkedCGIStrategy(ClientSocket *sok, char *buffer, size_t &len,
                                                   CgiRequest &cgireq, bool &pipeEof)
    : _sok(sok),
      _buffer(buffer),
      _len(len),
      _cgireq(cgireq),
      _pipeEof(pipeEof),
      _headerBuilt(false),
      _headerSended(0),
      _hasContentLength(false),
      _contentLength(0),
      _totalSent(0),
      _bodySended(0),
      _currentChunkSent(0),
      _terminatorSent(0)
{}

WriteChunkedCGIStrategy::~WriteChunkedCGIStrategy() {}

// ─── Header ──────────────────────────────────────────────────────────────────

void WriteChunkedCGIStrategy::_buildHeader()
{
    _headerBuilt = true;

    map<string, string> &headers = _cgireq.getrequestenv();

    // detect Content-Length
    map<string, string>::iterator clIt = headers.find("Content-Length");
    if (clIt != headers.end() && clIt->second != "0")
    {
        _hasContentLength = true;
        _contentLength = strtoul(clIt->second.c_str(), NULL, 10);
    }

    // status line
    _responseHeaderStr  = "HTTP/1.1 ";
    _responseHeaderStr += _cgireq.getStatusCode();
    _responseHeaderStr += " ";
    _responseHeaderStr += AMethod::getStatusMap()[_cgireq.getStatusCode()];
    _responseHeaderStr += "\r\n";

    // copy CGI headers; skip Content-Length when we will be chunking
    for (map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        if (!_hasContentLength && it->first == "Content-Length")
            continue;
        _responseHeaderStr += it->first + ": " + it->second + "\r\n";
    }

    if (!_hasContentLength)
        _responseHeaderStr += "Transfer-Encoding: chunked\r\n";

    _responseHeaderStr += "\r\n";
    
    DDEBUG("WriteChunkedCGIStrategy") << "Built header: " << _responseHeaderStr;
}

void WriteChunkedCGIStrategy::_sendHeader()
{
    int toSend = _responseHeaderStr.length() - _headerSended;
    int sent   = NetIO::Send(&_responseHeaderStr[_headerSended], toSend, _sok->GetFd());
    if (sent < 0)
    {
        _status = eWriteError;
        return;
    }
    _sok->SetSendStart(true);
    _headerSended += sent;
    _sok->UpdateTime();
}

// ─── CGI body (parsed alongside headers) ─────────────────────────────────────

void WriteChunkedCGIStrategy::_drainCgiBody()
{
    string &body = _cgireq.getBody();

    int maxStage = BUF_SIZE - CHUNK_HEADER_RESERVE - CHUNK_TRAILER_SIZE;
    int toStage  = body.length() - _bodySended;
    if (toStage > maxStage)
        toStage = maxStage;

    memcpy(_buffer + CHUNK_HEADER_RESERVE, &body[_bodySended], toStage);
    _len       = toStage;
    _bodySended += toStage;

    if (_bodySended >= body.length())
    {
        body.clear();
        _bodySended = 0;
    }
}

// ─── Content-Length path ─────────────────────────────────────────────────────

void WriteChunkedCGIStrategy::_streamDirect()
{
    int sent = NetIO::Send(_buffer + CHUNK_HEADER_RESERVE, _len, _sok->GetFd());
    if (sent < 0)
    {
        _status = eWriteError;
        return;
    }
    _len       -= sent;
    _totalSent += sent;
    if (_len > 0)
        memmove(_buffer + CHUNK_HEADER_RESERVE,
                _buffer + CHUNK_HEADER_RESERVE + sent,
                _len);
    if (_pipeEof && _len == 0)
        _status = eComplete;
    _sok->UpdateTime();
    _sok->SetSendStart(true);
}

// ─── Chunked encoding path ────────────────────────────────────────────────────

void WriteChunkedCGIStrategy::_sendChunked()
{
    char   sizeLine[CHUNK_HEADER_RESERVE];
    int    sizeLineLen = snprintf(sizeLine, sizeof(sizeLine), "%zx\r\n", (size_t)_len);
    char  *sendStart   = _buffer + CHUNK_HEADER_RESERVE - sizeLineLen;

    // idempotent on repeated calls (same _len → same frame)
    memcpy(sendStart, sizeLine, sizeLineLen);
    _buffer[CHUNK_HEADER_RESERVE + _len]     = '\r';
    _buffer[CHUNK_HEADER_RESERVE + _len + 1] = '\n';

    int totalFrame = sizeLineLen + _len + CHUNK_TRAILER_SIZE;
    int remaining  = totalFrame - _currentChunkSent;

    int sent = NetIO::Send(sendStart + _currentChunkSent, remaining, _sok->GetFd());
    if (sent < 0)
    {
        _status = eWriteError;
        return;
    }
    _currentChunkSent += sent;
    if (_currentChunkSent == (size_t)totalFrame)
    {
        _currentChunkSent = 0;
        _len = 0;           
    }
    _sok->UpdateTime();
    _sok->SetSendStart(true);
}

void WriteChunkedCGIStrategy::_sendTerminator()
{
    static char term[] = "0\r\n\r\n";
    int         total = 5;
    int sent = NetIO::Send(term + _terminatorSent, total - _terminatorSent, _sok->GetFd());
    if (sent < 0)
    {
        _status = eWriteError;
        return;
    }
    _terminatorSent += sent;
    if (_terminatorSent == (size_t)total)
        _status = eComplete;
    _sok->UpdateTime();
    _sok->SetSendStart(true);
}

// ─── Execute ──────────────────────────────────────────────────────────────────

int WriteChunkedCGIStrategy::Execute()
{
    if (_status != eContinue)
        return _status;

    _isBusy = true;
    // ── 1. Build header on first call ────────────────────────────────────────
    if (!_headerBuilt)
        _buildHeader();

    // ── 2. Send HTTP response header ─────────────────────────────────────────
    if (_headerSended < _responseHeaderStr.length())
    {
        _sendHeader();
        return _status;
    }

    // ── 3. Resume partial chunk frame ────────────────────────────────────────
    // A previous _sendChunked left _currentChunkSent > 0 — frame not fully sent yet.
    if (_currentChunkSent > 0)
    {
        _sendChunked();
        return _status;
    }

    // ── 4. Dispatch on available data ────────────────────────────────────────
    if (_len > 0)
    {
        // Buffer has staged data (from ReadPipeStrategy or _drainCgiBody).
        if (_hasContentLength)
            _streamDirect();
        else
            _sendChunked();
        return _status;
    }

    // ── 5. Stage next batch from CGI body (parsed alongside headers) ─────────
    if (_cgireq.getBody().length() > _bodySended)
    {
        _drainCgiBody();
        return _status;
    }

    // ── 6. All buffers empty — check EOF ─────────────────────────────────────
    if (_pipeEof)
    {
        if (_hasContentLength)
            _status = eComplete;
        else
            _sendTerminator();
        return _status;
    }

    _isBusy = false;
    // ── 7. Nothing to send yet — ReadPipeStrategy will wake us ───────────────
    return _status;
}
