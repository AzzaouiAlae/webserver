#include "SessionManager.hpp"

SessionManager *SessionManager::instance = NULL;

SessionManager::SessionManager() : SESSION_TIMEOUT(3600) {}

SessionManager::~SessionManager()
{
	for (map<string, Session *>::iterator it = sessions.begin(); it != sessions.end(); ++it)
	{
		delete it->second;
	}
	sessions.clear();
}

SessionManager *SessionManager::getInstance()
{
	if (instance == NULL)
	{
		instance = new SessionManager();
	}
	return instance;
}

string SessionManager::generateSessionId()
{
	stringstream ss;
	srand(time(NULL) + rand());
	for (int i = 0; i < 16; i++)
	{
		int random = rand() % 256;
		ss << hex << setw(2) << setfill('0') << random;
	}
	return ss.str();
}

Session *SessionManager::createSession()
{
	string sessionId = generateSessionId();
	Session *session = new Session(sessionId);
	sessions[sessionId] = session;
	DEBUG("SessionManager") << "Session created: id=" << sessionId;
	return session;
}

Session *SessionManager::getSession(const string &sessionId)
{
	map<string, Session *>::iterator it = sessions.find(sessionId);
	if (it == sessions.end())
	{
		DDEBUG("SessionManager") << "Session not found: id=" << sessionId;
		return NULL;
	}

	Session *session = it->second;

	// Check if expired
	time_t now = time(NULL);
	if (now - session->lastAccessedAt > SESSION_TIMEOUT)
	{
		DEBUG("SessionManager") << "Session expired and removed: id=" << sessionId;
		delete session;
		sessions.erase(it);
		return NULL;
	}

	// Update last accessed time
	session->lastAccessedAt = now;
	return session;
}

void SessionManager::destroySession(const string &sessionId)
{
	map<string, Session *>::iterator it = sessions.find(sessionId);
	if (it != sessions.end())
	{
		DEBUG("SessionManager") << "Session destroyed: id=" << sessionId;
		delete it->second;
		sessions.erase(it);
	}
}

void SessionManager::cleanupExpiredSessions()
{
	time_t now = time(NULL);
	int removed = 0;
	for (map<string, Session *>::iterator it = sessions.begin(); it != sessions.end();)
	{
		if (now - it->second->lastAccessedAt > SESSION_TIMEOUT)
		{
			delete it->second;
			sessions.erase(it++);
			removed++;
		}
		else
		{
			++it;
		}
	}
	if (removed > 0)
		DEBUG("SessionManager") << "Cleaned up " << removed << " expired session(s). Active sessions: " << sessions.size();
}

void SessionManager::setSessionData(const string &sessionId, const string &key, const string &value)
{
	Session *session = getSession(sessionId);
	if (session)
	{
		session->data[key] = value;
	}
}

string SessionManager::getSessionData(const string &sessionId, const string &key)
{
	Session *session = getSession(sessionId);
	if (session && session->data.find(key) != session->data.end())
	{
		return session->data[key];
	}
	return "";
}