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
    ~Response();

    
    Response& operator=(const Response& other);

    static std::string buildHttpResponse(const HttpResponse& response);

private:
    
};

#endif
