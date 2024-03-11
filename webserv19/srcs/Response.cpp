#include "../includes/Response.hpp"

Response::Response()
{
}

Response::Response(const Response& other)
{
    (void)other;
}

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

void Response::setCacheHeaders(HttpResponse& response, bool cacheEnabled, int maxAge)
{
    std::ostringstream cacheControlValue;
    
    if (cacheEnabled)
    {
        cacheControlValue << "max-age=" << maxAge;
        response.headers["Cache-Control"] = cacheControlValue.str();
        // Ajouter également Expires si nécessaire
    }
    else
    {
        response.headers["Cache-Control"] = "no-store";
        response.headers["Pragma"] = "no-cache";
        // Définir Expires dans le passé pour forcer l'expiration
        response.headers["Expires"] = "Thu, 01 Jan 1970 00:00:00 GMT";
    }
}

std::string Response::buildHttpResponse(const HttpResponse& response)
{
    LOG_INFO("Début de la construction de la réponse HTTP");

    bool cacheEnabled = true;
    int maxAge = 3600;
    Response::setCacheHeaders(const_cast<HttpResponse&>(response), cacheEnabled, maxAge);

    std::string httpVersion;
    if (response.httpVersion.empty())
    {
        httpVersion = "HTTP/1.1";
        LOG_INFO("La version HTTP est vide, utilisation de 'HTTP/1.1' par défaut");
    }
    else
    {
        httpVersion = response.httpVersion;
        LOG_INFO(std::string("Utilisation de la version HTTP spécifiée : ") + response.httpVersion);
    }

    std::ostringstream respStream;

    std::map<std::string, std::string>::const_iterator statusLine = response.headers.find("Status");
    if (statusLine != response.headers.end())
    {
        LOG_INFO("En-tête de statut CGI 'Status' trouvé, traitement en cours");

        std::istringstream statusStream(statusLine->second);
        std::string statusCode, statusMessage;
        std::getline(statusStream, statusCode, ' ');
        statusStream >> std::ws;
        std::getline(statusStream, statusMessage);
        respStream << httpVersion << " " << statusCode << " " << statusMessage << "\r\n";

        LOG_INFO(std::string("Statut CGI traité : ") + statusCode + " " + statusMessage);
    }
    else
    {
        respStream << httpVersion << " " << response.statusCode << " " << response.statusMessage << "\r\n";
        LOG_INFO("Aucun en-tête de statut CGI 'Status' spécifique trouvé, utilisation du statut par défaut");
    }

    LOG_INFO("Ajout des en-têtes HTTP à la réponse");

    for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
        if (it->first != "Status")
        {
            respStream << it->first << ": " << it->second << "\r\n";
            LOG_INFO(std::string("En-tête ajouté : ") + it->first + ": " + it->second);
        }
    }

    LOG_INFO("Ajout du corps de la réponse");

    respStream << "\r\n" << response.body;

    LOG_INFO("Fin de la construction de la réponse HTTP");

    return respStream.str();
}
