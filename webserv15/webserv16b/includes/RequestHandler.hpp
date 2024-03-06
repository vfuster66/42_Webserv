#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <cstdlib>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

#include "Structures.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"

class RequestHandler {
private:
    std::vector<ServerConfig> serverConfigs;
    int current_port;

    // Méthodes privées
    void parseBody(HttpRequest& request);
    std::string determineMimeType(const std::string& filePath);
    bool isMultipartFormData(const HttpRequest& request);
    std::string getBoundary(const std::string& contentType);
    void parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files);
    void saveFile(const std::string& content, const std::string& fileName);
    FilePart extractFilePart(const std::string& part);
    HttpResponse generateNotFoundResponse();
    std::string loadErrorPage(int statusCode);
    ServerConfig getServerConfigForPort(int port);
    bool isValidRequest(const HttpRequest& request);
    int extractPortFromHostHeader(const std::string& hostHeader);
    HttpResponse generateInternalServerErrorResponse();
    HttpResponse handleCgiRequest(const HttpRequest& request);
    bool isCgiRequest(const HttpRequest& request);
    std::string getScriptPathFromUri(const std::string& uri);

public:
    RequestHandler();
    void setServerConfigs(const std::vector<ServerConfig>& configs);
    void setServerPort(int port);

    // Méthodes de traitement des requêtes
    HttpRequest parseRequest(const std::string& requestText);
    HttpResponse handleRequest(const HttpRequest& request);
    HttpResponse handleGetRequestWithRedirection(const HttpRequest& request);
    HttpResponse handleGetRequest(const HttpRequest& request);
    HttpResponse handlePostRequest(const HttpRequest& request);
    HttpResponse handleDeleteRequest(const HttpRequest& request);
    

    // Utilitaires
    void parseRequestLine(const std::string& line, HttpRequest& request);
    void parseHeaders(std::istringstream& stream, HttpRequest& request);
    bool isDirectory(const std::string& path);
    std::map<std::string, std::string> parseFormData(const std::string& body);
    std::string getAbsolutePath(const std::string& uri, int port);
    std::string normalizePath(const std::string& path);
    bool performAction(const std::map<std::string, std::string>& formData);
    bool shouldBeRedirected(const std::string& uri);
};

#endif // REQUESTHANDLER_HPP