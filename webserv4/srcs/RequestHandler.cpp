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

std::string RequestHandler::loadErrorPage(int statusCode)
{
    std::ostringstream filePathStream;
    filePathStream << "www/errors/" << statusCode << ".html";
    std::string filePath = filePathStream.str();

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    }
    else
    {
        // Fallback si le fichier d'erreur n'existe pas, en utilisant std::ostringstream pour la conversion
        std::ostringstream errorMessage;
        errorMessage << "<html><body><h1>Error " << statusCode << "</h1></body></html>";
        return errorMessage.str();
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
        // Remplacement de filePath.back() par une approche compatible C++98
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
    // La section DELETE était répétée; j'ai supprimé la répétition.
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

    // Détermination de la longueur du corps de la réponse pour l'en-tête Content-Length
    std::ostringstream oss;
    oss << response.body.length();
    response.headers["Content-Length"] = oss.str();

    return response;
}

std::string RequestHandler::determineMimeType(const std::string& filePath)
{
    if (filePath.find(".html") != std::string::npos) return "text/html";
    if (filePath.find(".css") != std::string::npos) return "text/css";
    if (filePath.find(".js") != std::string::npos) return "application/javascript";
    if (filePath.find(".png") != std::string::npos) return "image/png";
    if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filePath.find(".gif") != std::string::npos) return "image/gif";
    if (filePath.find(".svg") != std::string::npos) return "image/svg+xml";
    if (filePath.find(".mp4") != std::string::npos) return "video/mp4";
    if (filePath.find(".webm") != std::string::npos) return "video/webm";
    if (filePath.find(".mp3") != std::string::npos) return "audio/mpeg";
    if (filePath.find(".wav") != std::string::npos) return "audio/wav";
    if (filePath.find(".pdf") != std::string::npos) return "application/pdf";
    if (filePath.find(".docx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (filePath.find(".json") != std::string::npos) return "application/json";
    if (filePath.find(".xml") != std::string::npos) return "application/xml";
    return "text/plain";
}
