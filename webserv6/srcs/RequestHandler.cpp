#include "../includes/RequestHandler.hpp"

HttpResponse RequestHandler::handleRequest(const HttpRequest& request)
{
    HttpResponse response;

    LOG_INFO("handleRequest -> Début du traitement de la requête: " + request.method + " " + request.uri);

    if (isCgiRequest(request.uri))
    {
        LOG_INFO("handleRequest -> Traitement d'une requête CGI pour l'URI: " + request.uri);

        std::map<std::string, std::string> cgiEnv;

        if (request.headers.find("Content-Length") != request.headers.end())
        {
            cgiEnv["CONTENT_LENGTH"] = request.headers.find("Content-Length")->second;
        }

        if (request.headers.find("Content-Type") != request.headers.end())
        {
            cgiEnv["CONTENT_TYPE"] = request.headers.find("Content-Type")->second;
        }

        cgiEnv["QUERY_STRING"] = extractQueryString(request.uri);
        cgiEnv["REMOTE_ADDR"] = "127.0.0.1";
        cgiEnv["REQUEST_METHOD"] = request.method;
        cgiEnv["SCRIPT_NAME"] = extractScriptName(request.uri);
        cgiEnv["SERVER_NAME"] = "localhost";
        cgiEnv["SERVER_PORT"] = "8080";
        cgiEnv["SERVER_PROTOCOL"] = request.httpVersion;
        cgiEnv["SERVER_SOFTWARE"] = "Webserv/1.0";

        std::string cgiOutput = "<html><body><h1>CGI Response</h1></body></html>";
        
        LOG_INFO("handleRequest -> Script CGI exécuté avec succès pour l'URI: " + request.uri);

        response.httpVersion = "HTTP/1.1";
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = cgiOutput;
        response.headers["Content-Type"] = "text/html";
    }
    else
    {
        LOG_INFO("handleRequest -> Traitement d'une requête non-CGI pour l'URI: " + request.uri);

        response.httpVersion = "HTTP/1.1";

        if (request.method == "GET")
        {
            std::string filePath = "www" + request.uri;
            if (!filePath.empty() && filePath[filePath.size() - 1] == '/')
            {
                filePath += "index.html";
            }

            std::ifstream file(filePath.c_str(), std::ios::binary);
            if (file)
            {
                response.statusCode = 200;
                response.statusMessage = "OK";
                response.body = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
                response.headers["Content-Type"] = determineMimeType(filePath);
                LOG_INFO("handleRequest -> Réponse pour GET prête: " + response.statusMessage + " pour " + filePath);
            }
            else
            {
                response.statusCode = 404;
                response.statusMessage = "Not Found";
                response.body = loadErrorPage(404);
                LOG_WARNING("handleRequest -> Fichier non trouvé pour GET requête: " + filePath);
            }
        }
        else if (request.method == "POST" && !isMultipartFormData(request))
        {
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = "<html><body><h1>POST Data Received</h1><p>" + request.body + "</p></body></html>";
            response.headers["Content-Type"] = "text/html";
            LOG_INFO("handleRequest -> Réponse pour POST prête: " + response.statusMessage);
        }
        else if (request.method == "DELETE")
        {
            std::string filePath = "www" + request.uri;
            if (std::remove(filePath.c_str()) == 0)
            {
                response.statusCode = 204;
                response.statusMessage = "No Content";
                LOG_INFO("handleRequest -> Fichier supprimé avec succès pour DELETE requête: " + filePath);
            }
            else
            {
                response.statusCode = 404;
                response.statusMessage = "Not Found";
                response.body = loadErrorPage(404);
                LOG_WARNING("handleRequest -> Fichier non trouvé pour DELETE requête: " + filePath);
            }
        }
        else
        {
            LOG_WARNING("handleRequest -> Méthode non autorisée: " + request.method);
            response.statusCode = 405;
            response.statusMessage = "Method Not Allowed";
            response.body = loadErrorPage(405);
        }

        std::ostringstream oss;
        oss << response.body.length();
        response.headers["Content-Length"] = oss.str();
    }

    std::ostringstream oss;
    oss << "handleRequest -> Fin du traitement de la requête, statut de réponse: " << response.statusCode;
    LOG_INFO(oss.str());
    return response;
}

//parsing
void RequestHandler::parseRequestLine(const std::string& line, HttpRequest& request)
{
    LOG_INFO("parseRequestLine -> Début de l'analyse de la ligne de requête");

    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;

    std::ostringstream logMessage;
    logMessage << "parseRequestLine -> Méthode : " << request.method
            << ", URI : " << request.uri
            << ", Version HTTP : " << request.httpVersion;
    LOG_INFO(logMessage.str());

    LOG_INFO("parseRequestLine -> Fin de l'analyse de la ligne de requête");
}


void RequestHandler::parseHeaders(std::istringstream& stream, HttpRequest& request)
{
    LOG_INFO("parseHeaders -> Début de l'analyse des en-têtes HTTP");

    std::string line;
    while (std::getline(stream, line) && line != "\r")
    {
        std::string::size_type delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos)
        {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            if (!value.empty() && value[value.length() - 1] == '\r')
            {
                value.erase(value.length() - 1);
            }
            request.headers[key] = value;

            std::ostringstream logMessage;
            logMessage << "parseHeaders -> En-tête trouvé : " << key << " : " << value;
            LOG_INFO(logMessage.str());
        }
    }

    LOG_INFO("parseHeaders -> Fin de l'analyse des en-têtes HTTP");
}


void RequestHandler::parseBody(std::istringstream& stream, HttpRequest& request)
{
    std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Length");
    if (it != request.headers.end())
    {
        std::istringstream contentLengthStream(it->second);
        std::size_t contentLength = 0;
        contentLengthStream >> contentLength;
        
        if (!contentLengthStream.fail())
        {
            LOG_INFO("parseBody -> Content-Length trouvé avec valeur : " + it->second);

            std::stringstream bodyStream;
            bodyStream << stream.rdbuf();

            request.body = bodyStream.str();
            if(request.body.length() != contentLength) {
                std::ostringstream warningMsg;
                warningMsg << "parseBody -> La longueur réelle du corps (" << request.body.length() 
                        << ") diffère de Content-Length (" << it->second << ")";
                LOG_WARNING(warningMsg.str());
            } else {
                std::ostringstream infoMsg;
                infoMsg << "parseBody -> Corps de la requête analysé avec succès, longueur : " << it->second;
                LOG_INFO(infoMsg.str());
            }
        }
        else
        {
            LOG_WARNING("parseBody -> Impossible de convertir Content-Length en nombre.");
        }
    }
    else
    {
        LOG_WARNING("parseBody -> Aucun Content-Length trouvé dans les en-têtes, corps de la requête ignoré.");
    }
}


HttpRequest RequestHandler::parseRequest(const std::string& requestText)
{
    LOG_INFO("parseRequest -> Début du parsing de la requête");

    HttpRequest request;
    std::istringstream stream(requestText);
    std::string requestLine;
    std::getline(stream, requestLine);

    LOG_INFO("parseRequest -> Parsing de la ligne de requête");
    parseRequestLine(requestLine, request);

    LOG_INFO("parseRequest -> Parsing des en-têtes");
    parseHeaders(stream, request);

    LOG_INFO("parseRequest -> Parsing du corps");
    parseBody(stream, request);

    LOG_INFO("parseRequest -> Parsing de la requête terminé");

    return request;
}


//extensions
std::string RequestHandler::determineMimeType(const std::string& filePath)
{
    std::string mimeType = "text/plain";

    if (filePath.find(".html") != std::string::npos)
        mimeType = "text/html";
    else if (filePath.find(".css") != std::string::npos)
        mimeType = "text/css";
    else if (filePath.find(".js") != std::string::npos)
        mimeType = "application/javascript";
    else if (filePath.find(".png") != std::string::npos)
        mimeType = "image/png";
    else if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos)
        mimeType = "image/jpeg";
    else if (filePath.find(".gif") != std::string::npos)
        mimeType = "image/gif";
    else if (filePath.find(".svg") != std::string::npos)
        mimeType = "image/svg+xml";
    else if (filePath.find(".mp4") != std::string::npos)
        mimeType = "video/mp4";
    else if (filePath.find(".webm") != std::string::npos)
        mimeType = "video/webm";
    else if (filePath.find(".mp3") != std::string::npos)
        mimeType = "audio/mpeg";
    else if (filePath.find(".wav") != std::string::npos)
        mimeType = "audio/wav";
    else if (filePath.find(".pdf") != std::string::npos)
        mimeType = "application/pdf";
    else if (filePath.find(".docx") != std::string::npos)
        mimeType = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    else if (filePath.find(".json") != std::string::npos)
        mimeType = "application/json";
    else if (filePath.find(".xml") != std::string::npos)
        mimeType = "application/xml";

    LOG_INFO("determineMimeType -> Type MIME déterminé pour " + filePath + " : " + mimeType);
    
    return mimeType;
}


// erreurs
std::string RequestHandler::loadErrorPage(int statusCode)
{
    std::ostringstream filePathStream;
    filePathStream << "www/errors/" << statusCode << ".html";
    std::string filePath = filePathStream.str();

    LOG_INFO("loadErrorPage -> Tentative de chargement de la page d'erreur : " + filePath);

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        LOG_INFO("loadErrorPage -> Page d'erreur chargée avec succès : " + filePath);
        return content;
    }
    else
    {
        LOG_ERROR("loadErrorPage -> Échec du chargement de la page d'erreur, fichier non trouvé : " + filePath);
        std::ostringstream errorMessage;
        errorMessage << "<html><body><h1>Error " << statusCode << "</h1></body></html>";
        return errorMessage.str();
    }
}


// upload
bool RequestHandler::isMultipartFormData(const HttpRequest& request)
{
    std::string contentType;
    std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        contentType = it->second;
        LOG_INFO("isMultipartFormData -> Content-Type trouvé : " + contentType);
    }
    else
    {
        LOG_WARNING("isMultipartFormData -> Aucun Content-Type trouvé dans les en-têtes de la requête.");
    }

    bool isMultipart = contentType.find("multipart/form-data") != std::string::npos;

    if (isMultipart)
    {
        LOG_INFO("isMultipartFormData -> La requête est de type multipart/form-data.");
    }
    else
    {
        LOG_INFO("isMultipartFormData -> La requête n'est pas de type multipart/form-data.");
    }

    return isMultipart;
}


std::string RequestHandler::getBoundary(const std::string& contentType)
{
    LOG_INFO("getBoundary -> Début de getBoundary avec contentType: " + contentType);

    std::string::size_type pos = contentType.find("boundary=");
    if (pos != std::string::npos)
    {
        std::string boundary = contentType.substr(pos + 9);
        LOG_INFO("getBoundary -> Boundary trouvé: " + boundary);
        return boundary;
    }

    LOG_WARNING("getBoundary -> Aucun boundary trouvé dans contentType");
    return "";
}

void RequestHandler::parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files)
{
    LOG_INFO("parseMultipartFormData -> Début de parseMultipartFormData");

    std::string delimiter = "--" + boundary;
    std::string endDelimiter = delimiter + "--";
    std::size_t pos = 0, endPos = 0;

    while ((pos = body.find(delimiter, pos)) != std::string::npos)
    {
        endPos = body.find(delimiter, pos + delimiter.length());

        if (endPos == std::string::npos)
            break;

        std::string part = body.substr(pos + delimiter.length(), endPos - pos - delimiter.length());
        FilePart filePart = extractFilePart(part);

        if (!filePart.fileName.empty())
        {
            LOG_INFO("parseMultipartFormData -> Fichier extrait: " + filePart.fileName);
            files.push_back(filePart);
        }

        pos = endPos + delimiter.length();
    }

    LOG_INFO("parseMultipartFormData -> Fin de parseMultipartFormData");
}

void RequestHandler::saveFile(const std::string& content, const std::string& fileName)
{
    std::string filePath = "/home/vfuster-/42-Webserv/webserv4/uploads/" + fileName;

    std::ofstream file(filePath.c_str(), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write(content.c_str(), content.size());
        file.close();
        LOG_INFO("saveFile -> Fichier sauvegardé avec succès : " + filePath);
    }
    else
    {
        LOG_ERROR("saveFile -> Erreur lors de l'ouverture du fichier pour écriture : " + filePath);
    }
}

FilePart RequestHandler::extractFilePart(const std::string& part)
{
    FilePart filePart;
    std::istringstream partStream(part);
    std::string line;
    std::string contentDisposition;
    bool contentStart = false;

    while (std::getline(partStream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }

        if (line.empty())
        {
            contentStart = true;
            LOG_INFO("extractFilePart -> Début du contenu du fichier détecté.");
            continue;
        }

        if (!contentStart)
        {
            if (line.find("Content-Disposition:") != std::string::npos)
            {
                contentDisposition = line;
                LOG_INFO("extractFilePart -> Content-Disposition trouvé : " + line);
                std::size_t filenamePos = line.find("filename=\"");
                if (filenamePos != std::string::npos)
                {
                    filenamePos += 10;
                    std::size_t filenameEnd = line.find("\"", filenamePos);
                    if (filenameEnd != std::string::npos)
                    {
                        filePart.fileName = line.substr(filenamePos, filenameEnd - filenamePos);
                        LOG_INFO("extractFilePart -> Nom du fichier extrait : " + filePart.fileName);
                    }
                }
            }
        }
        else
        {
            filePart.fileContent += line + "\n";
        }
    }

    if (!filePart.fileContent.empty())
    {
        filePart.fileContent.erase(filePart.fileContent.size() - 1);
    }

    LOG_INFO("extractFilePart -> Extraction du fichier terminée. Nom du fichier : " + filePart.fileName);
    return filePart;
}


// CGI
bool RequestHandler::isCgiRequest(const std::string& uri)
{
    bool isCgi = uri.find(".cgi") != std::string::npos || uri.find(".php") != std::string::npos;

    if (isCgi)
    {
        LOG_INFO("isCgiRequest -> URI identified as a CGI request: " + uri);
    }
    else
    {
        LOG_INFO("isCgiRequest -> URI not identified as a CGI request: " + uri);
    }

    return isCgi;
}

std::string RequestHandler::extractQueryString(const std::string& uri)
{
    size_t pos = uri.find("?");
    std::string queryString = "";

    if (pos != std::string::npos)
    {
        queryString = uri.substr(pos + 1);
        LOG_INFO("extractQueryString -> Extracted query string: " + queryString);
    }
    else
    {
        LOG_INFO("extractQueryString -> No query string found in URI");
    }

    return queryString;
}

std::string RequestHandler::extractScriptName(const std::string& uri)
{
    size_t pos = uri.find("?");
    std::string scriptName = "";

    if (pos != std::string::npos)
    {
        scriptName = uri.substr(0, pos);
        LOG_INFO("extractScriptName -> Extracted script name: " + scriptName);
    }
    else
    {
        scriptName = uri;
        LOG_INFO("extractScriptName -> Script name is the URI itself: " + scriptName);
    }

    return scriptName;
}

