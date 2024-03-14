#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

#include "RequestHandler.hpp"
#include "Logger.hpp"

class Response
{

public:
    
    Response();
    Response(const Response& other);
    virtual ~Response();

    Response& operator=(const Response& other);

    static std::string buildHttpResponse(const HttpResponse& response);

    static void setCacheHeaders(HttpResponse& response, bool cacheEnabled, int maxAge);

private:

};

#endif

