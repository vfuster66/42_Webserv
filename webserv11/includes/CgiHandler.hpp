#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fcntl.h>

#include "Logger.hpp"

class CgiHandler {
public:
    // Constructeurs et destructeur
    CgiHandler(const std::string& scriptPath, const std::map<std::string, std::string>& cgiEnv);
    CgiHandler(const CgiHandler& other);
    ~CgiHandler();

    // Opérateur d'assignation
    CgiHandler& operator=(const CgiHandler& other);

    // Méthodes
    std::string execute();
    std::string getScriptPath() const;

private:
    // Attributs
    std::string scriptPath;
    std::map<std::string, std::string> cgiEnv;

    // Méthodes privées
    void setEnvironmentVariables();
};

#endif

