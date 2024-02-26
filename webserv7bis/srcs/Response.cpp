#include "../includes/Response.hpp"

std::string Response::buildHttpResponse(const HttpResponse& response)
{
    std::ostringstream respStream;
    respStream << response.httpVersion << " " << response.statusCode << " " << response.statusMessage << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
        respStream << it->first << ": " << it->second << "\r\n";
    }

    respStream << "\r\n" << response.body;
    return respStream.str();
}
