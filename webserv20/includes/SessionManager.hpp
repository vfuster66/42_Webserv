#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <map>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include "Structures.hpp"
#include "Logger.hpp"

class SessionManager
{

public:

    std::string createSession();
    bool validateSession(const std::string& sessionId);
    void endSession(const std::string& sessionId);
    Session& getSession(const std::string& sessionId);

private:
    std::map<std::string, Session>  sessions;
    static int                      counter;

    std::string generateSessionId();

};

#endif
