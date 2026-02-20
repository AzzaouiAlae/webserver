#include "SessionManagement.hpp"

SessionManager::SessionManager(){};
string SessionManager::GenerateUID()
{
    string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.*?<>+_-=|~{}()[]";
    string result = "";
    for (int i = 0; i < 16; i++)
        result += charset[rand() % charset.length()];
    return result;
}

string      SessionManager::CreateSession(string username)
{
    string sid = GenerateUID();
    _active_sessions[sid] = username;
    return sid;
}

string      SessionManager::CreateResponseSession(string username)
{
    string sid = CreateSession(username);
    string cookiesResponse ;
    string path = "/";
    cookiesResponse += "SID=" + sid  + "; HttpOnly; Secure; Path=/; max" + ";";
    
}
void        SessionManager::parseCookies(string rawHeader, map<string, string>& cookies)
{
    size_t start = 0;
    size_t end = rawHeader.find(";");

    while (true) {
        string pair = rawHeader.substr(start, end - start);

        size_t sep = pair.find("=");
        if (sep != string::npos) {
            string key = pair.substr(0, sep);
            string value = pair.substr(sep + 1);

            if (!key.empty() && key[0] == ' ')
                key.erase(0, 1);

            cookies[key] = value;
        }

        if (end == string::npos)
            break;

        start = end + 1;
        end = rawHeader.find(";", start);
    }
}

bool SessionManager::IsValid(string sid)
{
    return _active_sessions.find(sid) != _active_sessions.end();
}
