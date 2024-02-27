#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "Structures.hpp"
#include "CgiHandler.hpp"
#include "Logger.hpp"

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
    bool isCgiRequest(const std::string& uri);
    HttpResponse generateNotFoundResponse();
    std::string loadErrorPage(int statusCode);
    ServerConfig getServerConfigForPort(int port);
    bool isValidRequest(const HttpRequest& request);
    int extractPortFromHostHeader(const std::string& hostHeader);
    HttpResponse generateInternalServerErrorResponse();

public:
    RequestHandler();
    void setServerConfigs(const std::vector<ServerConfig>& configs);
    void setServerPort(int port);

    // Méthodes de traitement des requêtes
    HttpRequest parseRequest(const std::string& requestText);
    HttpResponse handleRequest(const HttpRequest& request);
    HttpResponse handleGetRequest(const HttpRequest& request);
    HttpResponse handlePostRequest(const HttpRequest& request);
    HttpResponse handleDeleteRequest(const HttpRequest& request);
    //HttpResponse handleCgiRequest(const HttpRequest& request);

    // Utilitaires
    void parseRequestLine(const std::string& line, HttpRequest& request);
    void parseHeaders(std::istringstream& stream, HttpRequest& request);
    std::map<std::string, std::string> parseFormData(const std::string& body);
    bool performAction(const std::map<std::string, std::string>& formData);
};

#endif // REQUESTHANDLER_HPP


