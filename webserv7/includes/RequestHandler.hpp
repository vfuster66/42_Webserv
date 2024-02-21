#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class RequestHandler
{


public:

    virtual ~RequestHandler() {}
    virtual HttpResponse handleRequest(const HttpRequest& request) = 0;
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
