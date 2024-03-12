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
#include <dirent.h>
#include <cerrno>

#include "Structures.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"

class RequestHandler
{

public:
    RequestHandler();

    void setServerConfigs(const std::vector<ServerConfig>& configs);

    HttpResponse handleRequest(const HttpRequest& request);
    HttpRequest parseRequest(const std::string& requestText);

    std::string urlDecode(const std::string& str);
    std::string loadErrorPage(int statusCode);

private:

    std::vector<ServerConfig>   serverConfigs;
    int                         current_port;
    const ServerConfig* currentServerConfig;

    // Parsing de la requête
    void parseRequestLine(const std::string& line, HttpRequest& request);
    void parseHeaders(std::istringstream& stream, HttpRequest& request);
    void parseBody(HttpRequest& request);

    // Validation de la requête
    bool isValidRequest(const HttpRequest& request);

    // Gestion des extensions
    std::string determineMimeType(const std::string& filePath);

    // Gestion des pages d'erreurs
    HttpResponse generateNotFoundResponse();
    HttpResponse generateInternalServerErrorResponse();

    // Gestion des méthodes HTTP
    HttpResponse handleGetRequest(const HttpRequest& request);
    HttpResponse handlePostRequest(const HttpRequest& request);
    HttpResponse handleDeleteRequest(const HttpRequest& request);

    // Gestion des CGI
    HttpResponse handleCgiRequest(const HttpRequest& request);
    bool isCgiRequest(const HttpRequest& request);
    std::string getScriptPathFromUri(const std::string& uri);

    // Gestion des redirections
    HttpResponse handleGetRequestWithRedirection(const HttpRequest& request);

    // Gestion de la configuration
    void setServerPort(int port);
    int extractPortFromHostHeader(const std::string& hostHeader);
    ServerConfig getServerConfigForPort(int port);

    // Gestion des données de formulaire
    std::map<std::string, std::string> parseFormData(const std::string& body);
    bool performAction(const std::map<std::string, std::string>& formData);

    // Gestion du contenu
    bool isMultipartFormData(const HttpRequest& request);
    std::string getBoundary(const std::string& contentType);
    void parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files);
    std::string generateDirectoryListingHtml(const std::string& directoryPath);

    // Gestion des fichiers
    void saveFile(const std::string& content, const std::string& fileName);
    FilePart extractFilePart(const std::string& part);
    bool isDirectory(const std::string& path);
    std::string getAbsolutePath(const std::string& uri, int port);
    std::string normalizePath(const std::string& path);

    // Assistance
    std::string getUriBasePath();
    bool isMethodDenied(const std::string& method, const ServerConfig& serverConfig);

};

#endif
