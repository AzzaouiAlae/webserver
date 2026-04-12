#include "WriteBufferedCGIStrategy.hpp"
#include "AMethod.hpp"

WriteBufferedCGIStrategy::WriteBufferedCGIStrategy(ClientSocket *sok, char *buffer, size_t &len, 
                                                   CgiRequest &cgireq, ClientRequest &req, bool &pipeEof)
    : _sok(sok), _buffer(buffer), _len(len), _cgireq(cgireq), _req(req), _pipeEof(pipeEof),
      _internalState(eInit), _hasContentLength(false), 
      _tempFd(-1), _fileSize(0), _headerSent(0), _bodySended(0), _delegate(NULL)
{
    _memBuffer.reserve(MAX_MEM_BUFFER);
}

WriteBufferedCGIStrategy::~WriteBufferedCGIStrategy()
{
    if (_delegate)
        delete _delegate;
    if (_tempFd != -1)
        close(_tempFd);
}

void WriteBufferedCGIStrategy::_createTempFile()
{
    char temp_pattern[] = "/tmp/cgi_buf_XXXXXX";
    _tempFd = mkstemp(temp_pattern);
    if (_tempFd == -1) {
        _status = eWriteError;
        return;
    }
    unlink(temp_pattern);
}

void WriteBufferedCGIStrategy::_buildHeader(size_t contentLength)
{
    _responseHeaderStr  = "HTTP/1.1 ";
    _responseHeaderStr += _cgireq.getStatusCode();
    _responseHeaderStr += " ";
    _responseHeaderStr += AMethod::getStatusMap()[_cgireq.getStatusCode()];
    _responseHeaderStr += "\r\n";

    DDEBUG("WriteBufferedCGIStrategy") << _responseHeaderStr;
    map<string, string> &headers = _cgireq.getrequestenv();
    for (map<string, string>::iterator it = headers.begin(); it != headers.end(); ++it)
    {
        if (it->first != "Status" && it->first != "Content-Length")
            _responseHeaderStr += it->first + ": " + it->second + "\r\n";
    }

    if (!_hasContentLength)
    {
        ostringstream ss;
        ss << contentLength;
        _responseHeaderStr += "Content-Length: " + ss.str() + "\r\n";
    }
    else 
    {
        _responseHeaderStr += "Content-Length: " + headers["Content-Length"] + "\r\n";
    }

    _responseHeaderStr += "\r\n";
    DDEBUG("WriteBufferedCGIStrategy") << "Built header: " << _responseHeaderStr;
}

void WriteBufferedCGIStrategy::_accumulate()
{
    size_t availableMemSpace = MAX_MEM_BUFFER - _memBuffer.size();

    // 1. Fill Memory Buffer first
    if (availableMemSpace > 0)
    {
        size_t toCopy = std::min(availableMemSpace, _len);
        _memBuffer.insert(_memBuffer.end(), _buffer, _buffer + toCopy);
        
        // If we filled the memory and still have data left in _buffer, spill the rest to disk
        if (toCopy < _len)
        {
            if (_tempFd == -1) 
                _createTempFile();
            if (_tempFd != -1) {
                write(_tempFd, _buffer + toCopy, _len - toCopy);
                _fileSize += (_len - toCopy);
            }
        }
    }
    // 2. Memory is full, stream entirely to disk
    else 
    {
        if (_tempFd == -1) 
            _createTempFile();
        if (_tempFd != -1) {
            write(_tempFd, _buffer, _len);
            _fileSize += _len;
        }
    }

    _len = 0;
}

void WriteBufferedCGIStrategy::_streamDirect()
{
    // Drain CGI Body first (data caught during header parsing)
    if (_cgireq.getBody().length() > _bodySended)
    {
        char *data = &_cgireq.getBody()[_bodySended];
        size_t len = _cgireq.getBody().length() - _bodySended;
        int sent = NetIO::Send(data, len, _sok->GetFd());
        _sok->UpdateTime();

        if (sent <= 0) _status = eWriteError;
        else 
        {
            _bodySended += sent;
            _sok->SetSendStart(true);
        }
        return;
    }

    // Stream from buffer
    if (_len > 0)
    {
        int sent = NetIO::Send(_buffer, _len, _sok->GetFd());
        if (sent <= 0) {
            _status = eWriteError;
        } 
        else 
        {
            _sok->SetSendStart(true);
            _sok->UpdateTime();
            _len -= sent;
            if (_len > 0)
                memmove(_buffer, _buffer + sent, _len);
        }
    }

    if (_pipeEof && _len == 0 && _cgireq.getBody().length() == _bodySended)
        _status = eComplete;
}

void WriteBufferedCGIStrategy::_setupDelegate()
{
    size_t totalBodySize = _memBuffer.size() + _fileSize;
    _buildHeader(totalBodySize);
    
    _sendBuffers.push_back(make_pair((char*)_responseHeaderStr.c_str(), _responseHeaderStr.length()));
    
    // Create the correct Strategy
    if (_tempFd != -1 && _req.getMethod() != "HEAD") 
    {
        if (!_memBuffer.empty())
            _sendBuffers.push_back(make_pair(&_memBuffer[0], _memBuffer.size()));
        lseek(_tempFd, 0, SEEK_SET);
        _delegate = new FileStrategy(_sendBuffers, _tempFd, _fileSize, *_sok);
    } 
    else 
    {
        _delegate = new BuffersStrategy(_sendBuffers, *_sok);
    }
    
    _internalState = eSendingDelegate;
}

int WriteBufferedCGIStrategy::Execute()
{
    if (_status != eContinue)
        return _status;
    
    if (_internalState == eInit) 
    {
        map<string, string> &headers = _cgireq.getrequestenv();
        if (headers.find("Content-Length") != headers.end()) 
        {
            _hasContentLength = true;
            _buildHeader();
            _internalState = eSendingDirectHeader;
        } 
        else 
        {
            // Instantly inject any leftover body data from the CGI parsing phase
            if (_cgireq.getBody().length() > 0)
            {
                const char *leftover = _cgireq.getBody().c_str();
                _memBuffer.insert(_memBuffer.end(), leftover, leftover + _cgireq.getBody().length());
            }
            _internalState = eBuffering;
        }
    }

    // --- Direct Streaming Path (Has Content-Length) ---
    if (_internalState == eSendingDirectHeader)
    {
        int sent = NetIO::Send(&_responseHeaderStr[_headerSent], _responseHeaderStr.length() - _headerSent, _sok->GetFd());
        if (sent <= 0) 
        {
            _status = eWriteError;
        }
        else 
        {
            _headerSent += sent;
            _sok->UpdateTime();
            _sok->SetSendStart(true);
        }

        if (_headerSent == _responseHeaderStr.length())
        {
            if (_req.getMethod() == "HEAD")
                _status = eComplete;
            else
                _internalState = eStreamingDirect;
        }
        return _status;
    }

    if (_internalState == eStreamingDirect)
    {
        _streamDirect();
        return _status;
    }

    // --- Buffering Path (No Content-Length) ---
    if (_internalState == eBuffering) 
    {
        if (_len > 0)
            _accumulate(); 
            
        if (_pipeEof) {
            _setupDelegate();
            _status = _delegate->Execute();
        }
            
        return _status; 
    }

    // --- Delegate Path (Pushing buffered data to client) ---
    if (_internalState == eSendingDelegate) 
    {
        _status = _delegate->Execute();
    }
    
    return _status;
}
