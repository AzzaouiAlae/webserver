#pragma once
#include <iostream>
#include <map>
#include <vector>
// #include "../Headers.hpp"
#include <queue>

using namespace std;
#define MAXHEADERSIZE 8192
#define SESSION_TIMEOUT 10 // seconds
#define SESSION_TIMEOUT_STR "10" // seconds

struct Session {
    string id;
    map<string, string> data;
    time_t createdAt;
    time_t lastAccessedAt;
    
    Session(const string &sid);
    struct CompareTimeout {
		bool operator()(const Session *a, const Session *b) const ;
	};
    static priority_queue<Session*, vector<Session*>, Session::CompareTimeout> timeoutList;
};

class SessionManager {
private:
    map<string, Session*> sessions;
    static SessionManager* instance;
    
    SessionManager();
    string generateSessionId();
    
public:
    ~SessionManager();
    static SessionManager* getInstance();
    
    // Session operations
    Session* createSession();
    Session* getSession(const string &sessionId);
    void cleanupExpiredSessions();
    
    // Session data
    void setSessionData(const string &sessionId, const string &key, const string &value);
    string getSessionData(const string &sessionId, const string &key);
};