#pragma once

#include "RequestHandler.hpp"
#include <string>
#include <map>
#include <fstream>
#include <sstream>

// Structure représentant une requête HTTP simplifiée.
struct HttpRequest
{
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
};