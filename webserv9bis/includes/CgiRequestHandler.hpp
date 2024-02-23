#pragma once

#include "RequestHandler.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <errno.h>
#include <sstream>
#include <cstdlib> // Pour setenv
#include <map>
#include <string>

struct HttpResponse;
//struct HttpRequest;

class CgiRequestHandler : public RequestHandler
{
public:
    CgiRequestHandler();
    static void initCgiInterpreters();
    virtual HttpResponse handleRequest(const HttpRequest& request);
    ~CgiRequestHandler();

private:
    static std::string executeCgiScript(const HttpRequest& request);
    static void setupCgiEnvironment(const HttpRequest& request);
    static std::string getCgiInterpreter(const std::string& filePath);
    std::string extractQueryString(const std::string& uri);
    std::string extractScriptName(const std::string& uri);
    static std::string extractPathInfo(const std::string& uri);
    static std::string convertPathInfoToFilePath(const std::string& pathInfo);
    // Map des interpréteurs CGI par extension de fichier
    static std::map<std::string, std::string> cgiInterpreters;
};
 // CGIREQUESTHANDLER_HPP
