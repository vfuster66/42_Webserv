#include "RequestHandler.hpp"
#include <sstream>
#include <iostream>


void RequestHandler::parseRequestLine(const std::string& line, HttpRequest& request)
{
    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;
}

void RequestHandler::parseHeaders(std::istringstream& stream, HttpRequest& request) {
    std::string line;
    while (std::getline(stream, line) && line != "\r") {
        std::string::size_type delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            // Remove possible carriage return at the end of the value
            if (!value.empty() && value[value.length() - 1] == '\r') {
                value.erase(value.length() - 1);
            }
            request.headers[key] = value;
        }
    }
}

void RequestHandler::parseBody(std::istringstream& stream, HttpRequest& request) {
    if (request.headers.find("Content-Length") != request.headers.end()) {
        std::stringstream bodyStream;
        bodyStream << stream.rdbuf();
        request.body = bodyStream.str();
    }
}

HttpRequest RequestHandler::parseRequest(const std::string& requestText)
{
    HttpRequest request;
    std::istringstream stream(requestText);

    // Parse request line
    std::string requestLine;
    std::getline(stream, requestLine);
    parseRequestLine(requestLine, request);

    // Parse headers
    parseHeaders(stream, request);

    // Parse body
    parseBody(stream, request);

    return request;
}
std::string RequestHandler::loadErrorPage(int statusCode)
{
    std::ostringstream filePath;
    filePath << "www/errors/" << statusCode << ".html";  // Construit le chemin du fichier d'erreur

    std::ifstream file(filePath.str().c_str(), std::ifstream::binary);
    if (file)
    {
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    else
    {
        // Fallback si le fichier d'erreur n'existe pas
        return "<html><body><h1>Error " + std::to_string(statusCode) + "</h1></body></html>";
    }
}


HttpResponse RequestHandler::handleRequest(const HttpRequest& request)
{
    HttpResponse response;

    // Set the HTTP version and headers
    response.httpVersion = "HTTP/1.1";
    response.headers["Content-Type"] = "text/html";

    // Determine response status code and body based on the request
    if (request.method == "GET")
    {
        std::string filePath = "www" + request.uri;
        if (filePath.back() == '/')
        {
            filePath += "index.html";  // Serve index.html by default if a directory is requested
        }

        std::ifstream file(filePath.c_str(), std::ifstream::binary);
        if (file)
        {
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            response.headers["Content-Type"] = determineMimeType(filePath);
        }
        else
        {
            response.statusCode = 404;
            response.statusMessage = "Not Found";
            response.body = loadErrorPage(404);
        }
    }
    else if (request.method == "POST")
    {
        // Simple echo response for demonstration
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = "<html><body><h1>POST Data Received</h1><p>" + request.body + "</p></body></html>";
    }
    else if (request.method == "DELETE")
    {
    // Simple response for demonstration, actual resource deletion should be handled carefully
        response.statusCode = 202; // Accepted for processing, but the processing has not been completed.
        response.statusMessage = "Accepted";
        response.body = "<html><body><h1>DELETE Request Received</h1><p>Resource would be deleted: " + request.uri + "</p></body></html>";
    }
    else if (request.method == "DELETE")
    {
        // Supposons que vous ne preniez pas en charge DELETE pour le moment
        response.statusCode = 501;  // Not Implemented
        response.statusMessage = "Not Implemented";
        response.body = loadErrorPage(501);
    }
    else
    {
        // Gestion des autres méthodes non prises en charge
        response.statusCode = 405;  // Method Not Allowed
        response.statusMessage = "Method Not Allowed";
        response.body = loadErrorPage(405);
    }
    std::ostringstream oss;
    oss << response.body.length();
    response.headers["Content-Length"] = oss.str();
    return response;
}

std::string RequestHandler::determineMimeType(const std::string& filePath)
{
    // Simple MIME type determination based on file extension
    // This is a very basic implementation and should be extended based on actual requirements
    if (filePath.find(".html") != std::string::npos) return "text/html";
    if (filePath.find(".css") != std::string::npos) return "text/css";
    if (filePath.find(".js") != std::string::npos) return "application/javascript";
    if (filePath.find(".png") != std::string::npos) return "image/png";
    if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) return "image/jpeg";
    // Add more MIME types as needed
    return "text/plain";
}
