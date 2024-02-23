#pragma once

#include "RequestHandler.hpp"

class StaticRequestHandler : public RequestHandler {
public:
    virtual HttpResponse handleRequest(const HttpRequest& request);

};

