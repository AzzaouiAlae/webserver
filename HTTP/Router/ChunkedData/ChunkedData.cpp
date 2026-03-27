#include "ChunkedData.hpp"

ChunkedData::ChunkedData()
{
    _currentChunkSize = 0;
    _unchunkedSize = 0;
    _status = eParssingChunkSize;
    _totalUnchunkedSize = 0;
    _startIdx = 0;
}

void ChunkedData::_checkHexString(const string &data)
{
    for (size_t i = _unchunkedSize; i < data.length(); i++)
    {
        if (isdigit(data[i]) ||
            (data[i] >= 'a' && data[i] <= 'f') ||
            (data[i] >= 'A' && data[i] <= 'F'))
            continue;
        else if (data[i] == '\r' && i + 1 == data.length())
            continue;
        else
        {
            _status = eError;
            return;
        }
    }
}

void ChunkedData::_ParseChunkSize(string &data)
{
    size_t pos = data.find("\r\n", _unchunkedSize);
    if (pos != std::string::npos)
    {
        string chunkSizeStr = data.substr(_unchunkedSize, pos - _unchunkedSize);
        _currentChunkSize = strtol(chunkSizeStr.c_str(), NULL, 16);
        data.erase(_unchunkedSize, pos + 2 - _unchunkedSize);
        if (_currentChunkSize == 0)
        {
            _status = eComplete;
            data.erase(_unchunkedSize, 2);
        }
        else
            _status = eParsingChunk;
    }
    else if (data.length() - _unchunkedSize > 0) {
        _checkHexString(data);
    }
}

void ChunkedData::_decodeFirstChunk(string &data)
{
    ssize_t dataLen = data.length() - _unchunkedSize;
    if (_currentChunkSize + 2 <= dataLen)
    {
        _unchunkedSize += _currentChunkSize;
        _totalUnchunkedSize += _currentChunkSize;
        data.erase(_unchunkedSize, 2);
        _status = eChunkComplete;
    }
    else
    {
        ssize_t toUnchunk = min(_currentChunkSize, dataLen);
        _unchunkedSize += toUnchunk;
        _currentChunkSize -= toUnchunk;
        _totalUnchunkedSize += toUnchunk;
    }
}

void ChunkedData::SizeWritting(ssize_t size)
{
    if (_unchunkedSize >= size)
        _unchunkedSize -= size;
    else
        _unchunkedSize = 0;
}

int ChunkedData::UnchunkData(string &data)
{
    if (_status == eComplete || _status == eError)
        return _status == eComplete;
    do
    {
        if (_status == eChunkComplete)
        {
            _status = eParssingChunkSize;
        }
        if (_status == eParssingChunkSize)
        {
            _ParseChunkSize(data);
        }
        if (_status == eParsingChunk)
        {
            _decodeFirstChunk(data);
        }
    } while (_status == eChunkComplete);
    return _status;
}

ssize_t ChunkedData::GetCurrentChunkSize() const
{
    return _currentChunkSize;
}

string ChunkedData::_toHex(ssize_t num)
{
    stringstream ss;
    ss << hex << num;
    return ss.str();
}

ssize_t ChunkedData::ChunkeData(string &data, ssize_t startIdx, ssize_t size)
{
    string chunkSizeStr = _toHex(size);
    chunkSizeStr += "\r\n";
    _startIdx = startIdx - chunkSizeStr.length();
    for (int i = _startIdx; i < startIdx; i++) {
        chunkSizeStr[i] = data[i];
    }
    return size + chunkSizeStr.length();
}

ssize_t ChunkedData::GetUnchunkedSize() const
{
    return _unchunkedSize;
}

ssize_t ChunkedData::GetTotalUnchunkedSize() const
{
    return _totalUnchunkedSize;
}

int ChunkedData::GetStatus() const
{
    return _status;
}

ssize_t ChunkedData::GetStartIdx() const
{
    return _startIdx;
}
