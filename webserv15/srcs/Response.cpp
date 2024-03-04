#include "../includes/Response.hpp"

Response::Response()
{
}

Response::Response(const Response& other)
{
    (void)other;
}

// Destructeur
Response::~Response()
{
}

Response& Response::operator=(const Response& other)
{
    if (this != &other)
    {
    }
    return *this;
}

std::string Response::buildHttpResponse(const HttpResponse& response)
{
    LOG_INFO("buildHttpResponse -> Début de la construction de la réponse HTTP");

    std::string httpVersion;
    if (response.httpVersion.empty())
    {
        httpVersion = "HTTP/1.1";
        LOG_INFO("buildHttpResponse -> La version HTTP est vide, utilisation de 'HTTP/1.1' par défaut");
    }
    else
    {
        httpVersion = response.httpVersion;
        LOG_INFO(std::string("buildHttpResponse -> Utilisation de la version HTTP spécifiée : ") + response.httpVersion);
    }

    std::ostringstream respStream;

    std::map<std::string, std::string>::const_iterator statusLine = response.headers.find("Status");
    if (statusLine != response.headers.end())
    {
        LOG_INFO("buildHttpResponse -> En-tête de statut CGI 'Status' trouvé, traitement en cours");

        std::istringstream statusStream(statusLine->second);
        std::string statusCode, statusMessage;
        std::getline(statusStream, statusCode, ' ');
        statusStream >> std::ws;
        std::getline(statusStream, statusMessage);
        respStream << httpVersion << " " << statusCode << " " << statusMessage << "\r\n";

        LOG_INFO(std::string("buildHttpResponse -> Statut CGI traité : ") + statusCode + " " + statusMessage);
    } else {
        respStream << httpVersion << " " << response.statusCode << " " << response.statusMessage << "\r\n";
        LOG_INFO("buildHttpResponse -> Aucun en-tête de statut CGI 'Status' spécifique trouvé, utilisation du statut par défaut");
    }

    LOG_INFO("buildHttpResponse -> Ajout des en-têtes HTTP à la réponse");

    for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
        if (it->first != "Status") {
            respStream << it->first << ": " << it->second << "\r\n";
            LOG_INFO(std::string("buildHttpResponse -> En-tête ajouté : ") + it->first + ": " + it->second);
        }
    }

    LOG_INFO("buildHttpResponse -> Ajout du corps de la réponse");

    respStream << "\r\n" << response.body;

    LOG_INFO("buildHttpResponse -> Fin de la construction de la réponse HTTP");

    return respStream.str();
}
