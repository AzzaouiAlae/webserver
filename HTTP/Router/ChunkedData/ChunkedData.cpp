#include "ChunkedData.hpp"

ChunkedData::ChunkedData() : _status(eParsingSize), _remaining(0) {}

ChunkedData::Status ChunkedData::GetStatus() const { return _status; }

void ChunkedData::_parseSize(char *buf, size_t &r, size_t end)
{
    while (r < end)
    {
        char c = buf[r++];
        if (c == '\r')
            continue;
        if (c == '\n')
        {
            if (_partialSize.empty())
                return;

            size_t semiPos = _partialSize.find(';');
            if (semiPos != string::npos)
            {
                _partialSize = _partialSize.substr(0, semiPos);
            }

            char *err;
            _remaining = strtol(_partialSize.c_str(), &err, 16);
            _partialSize.clear();

            if (*err != '\0')
                _status = eError;
            else if (_remaining == 0)
            {
                _status = eComplete;
                DDEBUG("ChunkedData") << "ChunkedData: reached final chunk, total size=" << r;
            }
            else
                _status = eParsingBody;
            return;
        }
        _partialSize += c;
    }
}

void ChunkedData::_copyBody(char *buf, size_t &r, size_t &w, size_t end)
{
    size_t n = std::min((size_t)_remaining, end - r);
    if (w != r)
        memcpy(buf + w, buf + r, n);
    r += n;
    w += n;
    _remaining -= n;
    if (_remaining == 0)
        _status = eParsingSize;
}

ChunkedData::Result ChunkedData::Feed(char *buf, size_t len)
{
    size_t r = 0;
    size_t w = 0;

    while (r < len && _status != eComplete && _status != eError)
    {
        if (_status == eParsingSize)
            _parseSize(buf, r, len);
        else
            _copyBody(buf, r, w, len);
    }
    return (ChunkedData::Result){r, w};
}