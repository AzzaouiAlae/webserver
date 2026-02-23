#pragma once
#include "../Headers.hpp"

struct Session {
    string id;
    map<string, string> data;
    time_t createdAt;
    time_t lastAccessedAt;
    
    Session(const string &sid) : id(sid), createdAt(time(NULL)), lastAccessedAt(time(NULL)) {}
};

class SessionManager {
private:
    map<string, Session*> sessions;
    static SessionManager* instance;
    int SESSION_TIMEOUT; // seconds
    
    SessionManager();
    string generateSessionId();
    
public:
    ~SessionManager();
    static SessionManager* getInstance();
    
    // Session operations
    Session* createSession();
    Session* getSession(const string &sessionId);
    void destroySession(const string &sessionId);
    void cleanupExpiredSessions();
    
    // Session data
    void setSessionData(const string &sessionId, const string &key, const string &value);
    string getSessionData(const string &sessionId, const string &key);
};