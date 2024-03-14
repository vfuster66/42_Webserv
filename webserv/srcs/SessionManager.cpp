#include "../includes/SessionManager.hpp"

int SessionManager::counter = 0;

std::string SessionManager::generateSessionId()
{
    std::ostringstream oss;
    static bool initialized = false;

    if (!initialized)
    {
        std::srand(std::time(0));
        initialized = true;
    }

    long randomValue = std::rand();
    oss << "session_" << randomValue << "_" << ++counter;

    LOG_INFO("Session ID generated: " + oss.str());

    return oss.str();
}

std::string SessionManager::createSession()
{
    std::string sessionId = generateSessionId();
    sessions[sessionId] = Session();

    LOG_INFO("Session created with ID: " + sessionId);

    return sessionId;
}

bool SessionManager::validateSession(const std::string& sessionId)
{
    bool isValid = sessions.find(sessionId) != sessions.end();

    if (isValid)
    {
        LOG_INFO("Session validated: " + sessionId);
    }
    else
    {
        LOG_WARNING("Session validation failed: " + sessionId);
    }

    return isValid;
}

void SessionManager::endSession(const std::string& sessionId)
{
    sessions.erase(sessionId);

    LOG_INFO("Session ended: " + sessionId);
}

Session& SessionManager::getSession(const std::string& sessionId)
{
    std::map<std::string, Session>::iterator it = sessions.find(sessionId);

    if (it == sessions.end())
    {
        LOG_ERROR("Session not found: " + sessionId);
        throw std::runtime_error("Session not found");
    }

    LOG_INFO("Session retrieved: " + sessionId);

    return it->second;
}

void SessionManager::updateLastActivity(const std::string& sessionId)
{
    std::map<std::string, Session>::iterator it = sessions.find(sessionId);
    if (it != sessions.end())
    {
        it->second.lastActivity = std::time(0);

        std::ostringstream oss;
        oss << it->second.lastActivity;

        LOG_INFO("Last activity time updated to: " + oss.str() + " for session ID: " + sessionId);
    }
    else
    {
        LOG_WARNING("Attempted to update last activity for non-existent session ID: " + sessionId);
    }
}



void SessionManager::cleanupExpiredSessions(int expirationTimeInSeconds)
{
    std::time_t now = std::time(0);
    for (std::map<std::string, Session>::iterator it = sessions.begin(); it != sessions.end(); )
    {
        if (now - it->second.lastActivity > expirationTimeInSeconds)
        {
            LOG_INFO("Cleaning up expired session: " + it->first);
            sessions.erase(it++);
        } 
        else 
        {
            ++it;
        }
    }
}

