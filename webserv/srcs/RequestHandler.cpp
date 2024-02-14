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

    parseBody(stream, request);

    return request;
}

HttpResponse RequestHandler::handleRequest(const HttpRequest& request) {
    HttpResponse response;

    // Set the HTTP version and headers
    response.httpVersion = "HTTP/1.1";
    response.headers["Content-Type"] = "text/html";

    if (request.method == "GET")
    {
        // Logique pour gérer les requêtes GET
        if (request.uri == "/")
        {
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = "<html><body><h1>Welcome to our C++ Server</h1></body></html>";
        } else {
            // Gestion des cas où la ressource demandée n'est pas trouvée
            response.statusCode = 404;
            response.statusMessage = "Not Found";
            response.body = "<html><body><h1>404 Not Found</h1></body></html>";
        }
    } else if (request.method == "POST") {
        // Traitement spécifique pour POST
        response.statusCode = 200; // Ou un autre code approprié selon la logique de traitement
        response.statusMessage = "OK";
        response.body = "<html><body><h1>POST Data Processed</h1></body></html>";
    } else {
        // Réponse par défaut pour les méthodes non gérées
        response.statusCode = 405; // Méthode non autorisée
        response.statusMessage = "Method Not Allowed";
        response.body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    }

    // Définir l'en-tête Content-Length pour toutes les réponses
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
