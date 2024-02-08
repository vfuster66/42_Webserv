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
        std::string::size_type delimiterPos = line.find(": "); // Utilisation de std::string::size_type au lieu de auto
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

    // Optional: Parse body if needed

    return request;
}

HttpResponse RequestHandler::handleRequest(const HttpRequest& request) {
    HttpResponse response;

    // Set the HTTP version and headers
    response.httpVersion = "HTTP/1.1";
    response.headers["Content-Type"] = "text/html";

    // Determine response status code and body based on the request
    if (request.uri == "/") {
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = "<html><body><h1>Welcome to our C++ Server</h1></body></html>";
    } else {
        // Example of handling not found
        response.statusCode = 404;
        response.statusMessage = "Not Found";
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    }

    // Set Content-Length header using std::ostringstream
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
