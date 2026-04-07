#include "SubBufferUploadStrategy.hpp"

SubBufferUploadStrategy::SubBufferUploadStrategy()
{}

SubBufferUploadStrategy::~SubBufferUploadStrategy()
{}

void SubBufferUploadStrategy::SetBuffer(char *buffer, size_t bufferSize, int uploadFd)
{
    _buffer = buffer;
    _uploadFd = uploadFd;
    _bufferSize = bufferSize;
    _status = eContinue;
}

int SubBufferUploadStrategy::Execute()
{
    if (_status != eContinue)
        return _status;

    size_t totalWritten = 0;
    while (totalWritten < _bufferSize)
    {
        char *s = _buffer + totalWritten;
        size_t remaining = _bufferSize - totalWritten;
        ssize_t writeLen = write(_uploadFd, s, remaining);
        if (writeLen <= 0)
        {
            _status = eWriteError;
            return _status;
        }
        totalWritten += writeLen;
    }
    _status = eComplete;
    return _status;
}