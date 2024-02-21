#pragma once

#include "RequestHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <errno.h>
#include <sstream>
#include <cstdlib> // Pour setenv

class CgiRequestHandler : public RequestHandler
{
public:
    virtual HttpResponse handleRequest(const HttpRequest& request) override;

private:
    static std::string executeCgiScript(const HttpRequest& request);
    static void setupCgiEnvironment(const char* envp[]);
};
 // CGIREQUESTHANDLER_HPP
