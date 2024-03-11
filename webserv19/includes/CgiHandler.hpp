#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Structures.hpp"
#include "Logger.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <iostream>
#include <cerrno>
#include <cstdio>

class CgiHandler
{

public:

    CgiHandler(const std::string& scriptPath, const HttpRequest& request);
    ~CgiHandler();

    HttpResponse executeScript();

private:

    std::string                         scriptPath;
    HttpRequest                         request;
    std::map<std::string, std::string>  cgiEnvironment;

    void setupEnvironment();
    char** createEnvp(const std::map<std::string, std::string>& envMap);
    void freeEnvp(char** envp);
    HttpResponse parseCgiOutput(const std::string& output);

};

#endif

