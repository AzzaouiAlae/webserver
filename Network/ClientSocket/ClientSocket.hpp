#pragma once
#include "Headers.hpp"

class ClientSocket : public ISocket
{
    time_t _lastTime;
    int _timeoutStatus;
    int _timeout;
    bool _isKeepAlive;
    bool _closeConnection;
    bool _sendStart;
public:
    ClientSocket(int fd, int timeout);
    ~ClientSocket();
    bool ClosedConnection() const;
    void SetCloseConnection(bool val);
    bool IsTimeOut();
    void UpdateTime();
    time_t GetEndTime() const;
    void SetKeepAlive(bool val);
    bool IsKeepAlive() const;
    void SetTimeout(int timeout);
    int GetTimeout() const;
    void Handle();
    int GetTimeoutStatus() const;
    void SetTimeoutStatus(int status);
    void SetSendStart(bool val);
    bool GetSendStart() const;
};