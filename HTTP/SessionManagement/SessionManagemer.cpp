#include "SessionManager.hpp"
#include "sstream"
#include "Logging.hpp"

priority_queue<Session*, vector<Session*>, Session::CompareTimeout> Session::timeoutList;

SessionManager *SessionManager::instance = NULL;

SessionManager::SessionManager() {
	
}

Session::Session(const string &sid) : id(sid), createdAt(time(NULL)), lastAccessedAt(time(NULL)){
	timeoutList.push(this);
}

bool Session::CompareTimeout::operator()(const Session *a, const Session *b) const 
{
	return (a->lastAccessedAt > b->lastAccessedAt);
}

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
	cleanupExpiredSessions();

	map<string, Session *>::iterator it = sessions.find(sessionId);
	if (it == sessions.end())
	{
		DDEBUG("SessionManager") << "Session not found: id=" << sessionId;
		return NULL;
	}

	Session *session = it->second;

	// Check if expired
	time_t now = time(NULL);

	// Update last accessed time
	session->lastAccessedAt = now;
	return session;
}

void SessionManager::cleanupExpiredSessions()
{
	time_t now = time(NULL);
	while(Session::timeoutList.size())
	{
		Session *s = Session::timeoutList.top();

		if (now - s->lastAccessedAt > SESSION_TIMEOUT)
		{
			Session::timeoutList.pop();
			sessions.erase(sessions.find(s->id));
			delete s;
		}
		else
			break;
	}

	DEBUG("SessionManager") << "Cleaned up " << " expired session(s). Active sessions: " << Session::timeoutList.size();
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