#pragma once
#include "../Headers.hpp"
#include "../SocketIO/SocketIO.hpp"

enum estatus
{
    eSTART,
    eFORK,
    eSENDBUFFTOPIPE,
    eSENDSOCKETOPIPE,
    eFINISHWRITING,
    ePARSEDCGIHEADER,
    eSENDPIPETOSOCKET,
    eCOMPLETE
};

class Cgi
{
    Request     &_req;
    SocketIO    &_sok;
    pid_t       _pid;
    estatus     _status;        // ✅ was bool — now correct enum type
    bool        _headerParsed;  // ✅ renamed from _parsheader
    long        _time;
    bool        _eventexec;
    char        **_exec;
    size_t      _reqlen;

    string      _cgiResponseBuf; // buffer to accumulate CGI output
    string      _responseHeader; // parsed CGI response header
    string      _responseBody;   // CGI body to forward

    Cgi();
    void createChild();
    void writetocgi();
    void readfromcgi();
    void parseCgiHeader();

public:
    Cgi(Request &req, char** exec, SocketIO &sok);
    bool isExeted();
    bool isComplete();
    void Handle();
    void resetTime();
    long getTime();
    ~Cgi();
};