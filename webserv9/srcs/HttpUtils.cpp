#include "../includes/HttpUtils.hpp"

HttpRequest HttpUtils_parseRequest(const std::string& requestText)
{
   LOG_INFO("parseRequest -> Début du parsing de la requête");

    HttpRequest request;
    std::istringstream stream(requestText);
    std::string requestLine;
    std::getline(stream, requestLine);

    LOG_INFO("parseRequest -> Parsing de la ligne de requête");
    HttpUtils_parseRequestLine(requestLine, request);

    LOG_INFO("parseRequest -> Parsing des en-têtes");
    HttpUtils_parseHeaders(stream, request);

    LOG_INFO("parseRequest -> Parsing du corps");
    HttpUtils_parseBody(stream, request);

    LOG_INFO("parseRequest -> Parsing de la requête terminé");

    return request;
}

void HttpUtils_parseRequestLine(const std::string& line, HttpRequest& request)
{
    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;
}

void HttpUtils_parseHeaders(std::istringstream& stream, HttpRequest& request)
{
    std::string line;
    while (std::getline(stream, line) && line != "\r")
    {
        std::string::size_type delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos)
	{
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            // Remove possible carriage return at the end of the value
            if (!value.empty() && value[value.length() - 1] == '\r')
	    {
                value.erase(value.length() - 1);
            }
            request.headers[key] = value;
        }
    }
}

void HttpUtils_parseBody(std::istringstream& stream, HttpRequest& request)
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

std::string HttpUtils_determineMimeType(const std::string& filePath)
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

std::string HttpUtils_loadErrorPage(int statusCode)
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
//cookies

std::map<std::string, std::string> HttpUtils_extractCookies(const HttpRequest& request)
{
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string>::const_iterator it = request.headers.find("Cookie");
    if (it != request.headers.end())
    {
        std::stringstream ss(it->second);
        std::string cookie;
        while (getline(ss, cookie, ';'))
        {
            size_t pos = cookie.find('=');
            if (pos != std::string::npos)
            {
                std::string name = cookie.substr(0, pos);
                std::string value = cookie.substr(pos + 1);
                cookies[name] = value;
            }
        }
    }
    return cookies;
}
void HttpUtils_setCookie(HttpResponse& response, const std::string& name, const std::string& value, int maxAge)
{
    std::stringstream cookie;
    cookie << name << "=" << value << "; Max-Age=" << maxAge;
    response.headers["Set-Cookie"] = cookie.str();
}