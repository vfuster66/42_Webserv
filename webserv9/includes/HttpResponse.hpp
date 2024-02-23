#pragma once

#include "HttpRequest.hpp"
#include <string>
#include <map>
#include <fstream>
#include <sstream>

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