#include "../Headers.hpp"

class SessionManager
{
    map<string, string> _active_sessions;

    string GenerateUID();

    public:
        SessionManager();

        string      CreateSession(string username);
        string      CreateResponseSession(string username);
        void        parseCookies(string rawHeader, map<string, string>& cookies);

        bool        IsValid(string sid);
};
