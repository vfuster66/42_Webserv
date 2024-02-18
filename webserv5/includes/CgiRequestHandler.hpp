#pragma once

#include "RequestHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <errno.h>

class CgiRequestHandler : public RequestHandler
{
public:
    virtual HttpResponse handleRequest(const HttpRequest& request) override;
};
 // CGIREQUESTHANDLER_HPP
