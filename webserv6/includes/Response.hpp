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
    static std::string buildHttpResponse(const HttpResponse& response);

};

#endif