#include "../includes/RequestHandler.hpp"

std::string RequestHandler::getHeader(const HttpRequest& request, const std::string& key)
{
        std::map<std::string, std::string>::const_iterator it = request.headers.find(key);
        if (it != request.headers.end())
        {
            return it->second;
        }
        return "";
}