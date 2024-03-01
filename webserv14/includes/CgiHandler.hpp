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

class CgiHandler {
public:
    CgiHandler(const std::string& scriptPath, const HttpRequest& request);
    ~CgiHandler();

    HttpResponse executeScript();

private:
    std::string scriptPath;
    HttpRequest request;
    std::map<std::string, std::string> cgiEnvironment;

    void setupEnvironment();
    std::string removeCarriageReturn(const std::string& input);
    HttpResponse parseCgiOutput(const std::string& output);
};

#endif