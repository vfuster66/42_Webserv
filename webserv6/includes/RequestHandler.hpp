#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

#include "CgiHandler.hpp"
#include "Logger.hpp" 


// Structure représentant une requête HTTP simplifiée.
struct HttpRequest
{
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
};

// Structure représentant une réponse HTTP simplifiée.
struct HttpResponse
{
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }
};

struct FilePart {
    std::string fileName;
    std::string fileContent;
};

class RequestHandler
{

private:

    void parseBody(std::istringstream& stream, HttpRequest& request);
    std::string determineMimeType(const std::string& filePath);
    bool isMultipartFormData(const HttpRequest& request);
    std::string getBoundary(const std::string& contentType);
    void parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files);
    void saveFile(const std::string& content, const std::string& fileName);
    FilePart extractFilePart(const std::string& part);
    bool isCgiRequest(const std::string& uri);
    std::string extractQueryString(const std::string& uri);
    std::string extractScriptName(const std::string& uri);

public:
    HttpRequest parseRequest(const std::string& requestText);
    HttpResponse handleRequest(const HttpRequest& request);
    static std::string getHeader(const HttpRequest& request, const std::string& key)
    {
        std::map<std::string, std::string>::const_iterator it = request.headers.find(key);
        if (it != request.headers.end())
        {
            return it->second;
        }
        return "";
    }
    std::string loadErrorPage(int statusCode);
    void parseRequestLine(const std::string& line, HttpRequest& request);
    void parseHeaders(std::istringstream& stream, HttpRequest& request);

};

#endif
