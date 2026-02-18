#include "../Headers.hpp"

class SessionManager
{
    std::map<std::string, std::string> _active_sessions;

    std::string GenerateUID();

    public:
        SessionManager() {}

        // Called when a user logs in via POST/CGI
        // Generates a new ID and saves it in the map
        std::string CreateSession(std::string username);
        void        parseCookies(std::string rawHeader);

        // Called during every Request to see if the user is "logged in"
        bool IsValid(std::string sid);
};
