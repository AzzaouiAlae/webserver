#include "Cgi_Cgi.hpp"

Cgi::Cgi(Request &req, char** exec, SocketIO &sok) : _req(req), _sok(sok)
{
    _exec       = exec;
    _reqlen     = 0;
    _status     = eSTART;
    _eventexec  = false;
    _headerParsed = false;
    _pid        = -1;
}

// ─────────────────────────────────────────────
//  Lifecycle helpers
// ─────────────────────────────────────────────

bool Cgi::isExeted()
{
    int status;
    if (waitpid(_pid, &status, WNOHANG) != _pid)
        return false;
    return true;
}

bool Cgi::isComplete()
{
    return _status == eCOMPLETE;
}

void Cgi::resetTime() { _time = Utility::CurrentTime(); }
long Cgi::getTime()   { return _time; }

// ─────────────────────────────────────────────
//  createChild — fork + exec the CGI process
// ─────────────────────────────────────────────
void Cgi::createChild()
{
    _pid = fork();
    if (_pid == -1)
        Error::ThrowError("Fork Failed");
    else if (_pid == 0) // child
    {
        // Set CGI environment variables from the request
        Environment::CreateEnv(_req.getrequestenv());

        // pipefd[1] = write end  → CGI's stdin  (server writes body here)
        // pipefd[0] = read end   → CGI's stdout (server reads response from here)
        // ✅ dup2(oldfd, newfd): replace stdin with pipe read end
        dup2(_sok.pipefd[1], STDIN_FILENO);   // CGI reads its stdin from pipefd[1]
        close(_sok.pipefd[1]);

        dup2(_sok.pipefd[0], STDOUT_FILENO);  // CGI writes its stdout to pipefd[0]
        close(_sok.pipefd[0]);

        execve(_exec[0], _exec, environ);
        exit(1); // execve failed
    }
    // parent continues
    _status = eFORK;
}

// ─────────────────────────────────────────────
//  writetocgi — send request body to CGI stdin
//  Uses: _sok.SendBuffToPipe()  → buffered body from memory
//        _sok.SendSocketToPipe() → remaining body still in socket
// ─────────────────────────────────────────────
void Cgi::writetocgi()
{
    // Only write if there is a body to send
    if (_req.getContentLen() == 0)
    {
        _status = eFINISHWRITING;
        return;
    }

    if (_status == eFORK && !_eventexec)
    {
        // Step 1: send the body already buffered in Request into pipe
        string &body = _req.getBody();
        int len = _sok.SendBuffToPipe((void *)body.c_str(), body.size());
        if (len > 0)
            _reqlen += len;
        _status = eSENDBUFFTOPIPE;
    }
    else if (_status == eSENDBUFFTOPIPE && !_eventexec)
    {
        // Step 2: splice remaining body from the clientsoc ket into the pipe
        int len = _sok.SendSocketToPipe();
        if (len > 0)
            _reqlen += len;

        if (_reqlen >= _req.getContentLen())
        {
            _status = eFINISHWRITING; // ✅ was _status == (assignment bug fixed)
        }
        else
        {
            _status = eSENDSOCKETOPIPE;
        }
    }
    else if (_status == eSENDSOCKETOPIPE && !_eventexec)
    {
        // Continue streaming from socket to pipe
        int len = _sok.SendSocketToPipe();
        if (len > 0)
            _reqlen += len;

        if (_reqlen >= _req.getContentLen())
            _status = eFINISHWRITING;
    }
}

// ─────────────────────────────────────────────
//  parseCgiHeader — extract Status + Content-Type
//  from the raw CGI output buffer
// ─────────────────────────────────────────────
void Cgi::parseCgiHeader()
{
    // CGI output looks like:
    //   Content-Type: text/html\r\n
    //   Status: 200 OK\r\n        (optional)
    //   \r\n
    //   <body>
    size_t headerEnd = _cgiResponseBuf.find("\r\n\r\n");
    if (headerEnd == string::npos)
        headerEnd = _cgiResponseBuf.find("\n\n");
    if (headerEnd == string::npos)
        return; // not enough data yet

    _responseHeader = _cgiResponseBuf.substr(0, headerEnd);
    _responseBody   = _cgiResponseBuf.substr(headerEnd + 4); // skip \r\n\r\n
    _headerParsed   = true;
    _status = ePARSEDCGIHEADER;
}

// ─────────────────────────────────────────────
//  readfromcgi — read CGI stdout, forward to client
//  Uses: read(pipefd[0])        → get raw CGI output
//        _sok.SendPipeToSock()  → forward pipe content to client socket
// ─────────────────────────────────────────────
void Cgi::readfromcgi()
{
    if (_status == eFINISHWRITING && !_headerParsed)
    {
        // Read raw output from CGI into our buffer
        char buf[4096];
        int len = read(_sok.pipefd[0], buf, sizeof(buf));
        if (len > 0)
        {
            _cgiResponseBuf.append(buf, len);
            parseCgiHeader(); // try to find end of CGI headers
        }
        else if (len == 0 && isExeted())
        {
            // CGI exited without sending valid headers → 502 Bad Gateway
            // TODO: call HandelErrorPages("502") via a callback or store error code
            _status = eCOMPLETE;
        }
    }
    else if (_status == ePARSEDCGIHEADER)
    {
        // Forward the CGI response (header + body) to the client socket
        // The CGI already wrote HTTP-style headers, so we wrap them in an
        // HTTP/1.1 response using _sok.Send() or SendPipeToSock()
        int len = _sok.SendPipeToSock();
        if (len == 0 && isExeted())
            _status = eSENDPIPETOSOCKET;
    }
    else if (_status == eSENDPIPETOSOCKET)
    {
        // Drain anything remaining in the pipe
        int len = _sok.SendPipeToSock();
        if (len == 0)
            _status = eCOMPLETE;
    }
}

// ─────────────────────────────────────────────
//  Handle — called by HTTPContext on each epoll event
// ─────────────────────────────────────────────
void Cgi::Handle()
{
    _eventexec = false;

    if (_status == eSTART)
        createChild();

    if (_status < eFINISHWRITING)
        writetocgi();

    if (_status >= eFINISHWRITING && _status < eCOMPLETE)
        readfromcgi();
}

Cgi::~Cgi()
{
    if (_pid > 0)
        waitpid(_pid, NULL, WNOHANG);
}