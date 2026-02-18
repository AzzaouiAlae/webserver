#include "Cookies.hpp"

string SessionManager::GenerateUID()
{
    string charset = "abcdefghijklmnopqrstuvwxyz0123456789";
    string result = "";
    for (int i = 0; i < 16; i++)
        result += charset[rand() % charset.length()];
    return result;
}

string SessionManager::CreateSession(string username)
{
    string sid = GenerateUID();
    _active_sessions[sid] = username;
    return sid;
}

void SessionManager::parseCookies(string rawHeader)
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

            _active_sessions[key] = value;
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