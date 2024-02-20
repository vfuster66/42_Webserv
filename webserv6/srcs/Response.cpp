#include "../includes/Response.hpp"

std::string Response::buildHttpResponse(const HttpResponse& response)
{
    LOG_INFO("buildHttpResponse -> Début de la construction de la réponse HTTP");

    std::ostringstream respStream;
    respStream << response.httpVersion << " " << response.statusCode << " " << response.statusMessage << "\r\n";
    
    LOG_INFO("buildHttpResponse -> Ajout des en-têtes HTTP à la réponse");

    for (std::map<std::string, std::string>::const_iterator it = response.headers.begin();
            it != response.headers.end(); ++it)
    {
        respStream << it->first << ": " << it->second << "\r\n";
    }
    
    LOG_INFO("buildHttpResponse -> Ajout du corps de la réponse");

    respStream << "\r\n" << response.body;

    LOG_INFO("buildHttpResponse -> Fin de la construction de la réponse HTTP");

    return respStream.str();
}


