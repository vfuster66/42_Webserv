#pragma once

#include <string>
#include <map>
#include <fstream>
#include <sstream>
//#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

struct HttpRequest;

class RequestHandler
{


public:

    virtual ~RequestHandler() {}
    virtual HttpResponse handleRequest(const HttpRequest& request) = 0;
    static std::string getHeader(const HttpRequest& request, const std::string& key);

};

