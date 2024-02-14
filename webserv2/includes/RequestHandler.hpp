#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <map>

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

class RequestHandler
{

private:
    void parseRequestLine(const std::string& line, HttpRequest& request);
    void parseHeaders(std::istringstream& stream, HttpRequest& request);
    std::string determineMimeType(const std::string& filePath);

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

};
#endif
