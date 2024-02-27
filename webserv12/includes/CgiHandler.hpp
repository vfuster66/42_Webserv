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
#include <map>


#include "Logger.hpp"
#include "Structures.hpp"

extern char **environ;

class CgiHandler {
public:
    // Constructeurs et destructeur
    CgiHandler();
    CgiHandler(const CgiHandler& other);
    ~CgiHandler();
    void initCgiInterpreters();
    static std::map<std::string, std::string> cgiInterpreters;
    // Opérateur d'assignation
    CgiHandler& operator=(const CgiHandler& other);
    void setScriptPath(const std::string& path) {
        scriptPath = path;
    }
    void setCgiEnv(const std::map<std::string, std::string>& env) {
        cgiEnv = env;
    }
    std::string execute(const HttpRequest& request);

    HttpResponse executeCgi(const HttpRequest& request);
    
private:
    std::string scriptPath;
    std::map<std::string, std::string> cgiEnv;
    HttpResponse handleCgiRequest(const HttpRequest& request);
    static std::string executeCgiScript(const HttpRequest& request);
    static void setupCgiEnvironment(const HttpRequest& request);
    static std::string getCgiInterpreter(const std::string& filePath);
    static std::string extractQueryString(const std::string& uri);
    static std::string extractScriptName(const std::string& uri);
    static std::string extractPathInfo(const std::string& uri);
    static std::string convertPathInfoToFilePath(const std::string& pathInfo);

};

#endif

