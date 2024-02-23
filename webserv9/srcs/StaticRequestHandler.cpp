#include "../includes/StaticRequestHandler.hpp"
#include "HttpUtils.hpp"
#include <sstream>
#include <iostream>


HttpResponse StaticRequestHandler::handleRequest(const HttpRequest& request)
{
    HttpResponse response;

    // Set the HTTP version and headers
    response.httpVersion = "HTTP/1.1";
    response.headers["Content-Type"] = "text/html";

    // Determine response status code and body based on the request
    if (request.method == "GET")
    {
        std::string filePath = "www" + request.uri;

        if (!filePath.empty() && filePath[filePath.size() - 1] == '/')
        {
            filePath += "index.html";
        }

        std::ifstream file(filePath.c_str(), std::ios::binary);
        if (file)
        {
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            response.headers["Content-Type"] = HttpUtils_determineMimeType(filePath);
        }
        else
        {
            response.statusCode = 404;
            response.statusMessage = "Not Found";
            response.body = HttpUtils_loadErrorPage(404);
        }
    }
    else if (request.method == "POST")
    {
        // Simple echo response for demonstration
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = "<html><body><h1>POST Data Received</h1><p>" + request.body + "</p></body></html>";
    }
    // La section DELETE était répétée; j'ai supprimé la répétition.
    else if (request.method == "DELETE")
    {
        // Supposons que vous ne preniez pas en charge DELETE pour le moment
        response.statusCode = 501;  // Not Implemented
        response.statusMessage = "Not Implemented";
        response.body = HttpUtils_loadErrorPage(501);
    }
    else
    {
        // Gestion des autres méthodes non prises en charge
        response.statusCode = 405;  // Method Not Allowed
        response.statusMessage = "Method Not Allowed";
        response.body = HttpUtils_loadErrorPage(405);
    }

    // Détermination de la longueur du corps de la réponse pour l'en-tête Content-Length
    std::ostringstream oss;
    oss << response.body.length();
    response.headers["Content-Length"] = oss.str();

    return response;
}


