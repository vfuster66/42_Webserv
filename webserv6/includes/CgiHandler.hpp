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
    CgiHandler(const std::string& scriptPath, const std::map<std::string, std::string>& cgiEnv);
    std::string execute();
    std::string getScriptPath() const;

private:
    std::string scriptPath;
    std::map<std::string, std::string> cgiEnv;

    void setEnvironmentVariables();
};

#endif
