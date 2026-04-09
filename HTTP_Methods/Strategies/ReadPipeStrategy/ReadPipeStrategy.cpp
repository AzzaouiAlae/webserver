#include "ReadPipeStrategy.hpp"

ReadPipeStrategy::ReadPipeStrategy(int pipeFd, char *buffer, size_t &len,
                                   CgiRequest &cgireq, bool &headerParsed, bool chunkedMode)
    : _pipeFd(pipeFd),
      _buffer(buffer),
      _len(len),
      _cgireq(cgireq),
      _headerParsed(headerParsed),
      _chunkedMode(chunkedMode),
      _totalRead(0)
{
    
}

ReadPipeStrategy::~ReadPipeStrategy() {}

// ─── Phase 1 ─────────────────────────────────────────────────────────────────

void ReadPipeStrategy::_readHeader()
{
    int len = read(_pipeFd, _buffer, BUF_SIZE);
    if (len < 0)
    {
        _status = eReadError;
        return;
    }
    if (len == 0)
    {
        _status = eComplete;
        return;
    }
    _totalRead += len;
    
    if (_cgireq.isComplete(_buffer, len))
        _headerParsed = true;
}

// ─── Phase 2 ─────────────────────────────────────────────────────────────────

void ReadPipeStrategy::_readBody()
{
    if (_chunkedMode)
    {
        if (_len > 0)
            return;

        int toRead = BUF_SIZE - CHUNK_HEADER_RESERVE - CHUNK_TRAILER_SIZE;
        int len = read(_pipeFd, _buffer + CHUNK_HEADER_RESERVE, toRead);
        if (len < 0)
        {
            _status = eReadError;
            return;
        }
        if (len == 0)
        {
            _status = eComplete;
            return;
        }
        _len = len;
        _totalRead += len;
    }
    else
    {
        int toRead = BUF_SIZE - _len;
        if (toRead == 0)
            return;

        int len = read(_pipeFd, _buffer + _len, toRead);
        if (len < 0)
        {
            _status = eReadError;
            return;
        }
        if (len == 0)
        {
            _status = eComplete;
            return;
        }
        _len += len;
        _totalRead += len;
    }
}

// ─── Execute ──────────────────────────────────────────────────────────────────

int ReadPipeStrategy::Execute()
{
    if (_status == eComplete || _status == eReadError)
        return _status;

    if (!_headerParsed)
        _readHeader();
    else
        _readBody();

    return _status;
}
