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

    if (isValid) {
        LOG_INFO("Session validated: " + sessionId);
    } else {
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

