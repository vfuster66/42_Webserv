#include "../includes/HttpUtils.hpp"


HttpRequest HttpUtils_parseRequest(const std::string& requestText)
{
   LOG_INFO("parseRequest -> Début du parsing de la requête");

    HttpRequest request;
    std::istringstream stream(requestText);
    std::string requestLine;
    std::getline(stream, requestLine);

    if (!std::getline(stream, requestLine) || requestLine.empty()) {
        // Si la ligne de requête est vide ou ne peut pas être lue, log une erreur et retourne une requête vide
        LOG_ERROR("parseRequest -> Ligne de requête vide ou non lisible");
        return request; // Retourne une requête vide pour éviter tout traitement ultérieur
    }


    LOG_INFO("parseRequest -> Parsing de la ligne de requête");
    HttpUtils_parseRequestLine(requestLine, request);

    LOG_INFO("parseRequest -> Parsing des en-têtes");
    HttpUtils_parseHeaders(stream, request);

    LOG_INFO("parseRequest -> Parsing du corps");
    HttpUtils_parseBody(request);

    LOG_INFO("parseRequest -> Parsing de la requête terminé");

    return request;
}

void HttpUtils_parseRequestLine(const std::string& line, HttpRequest& request)
{
    LOG_INFO("parseRequestLine -> Début de l'analyse de la ligne de requête");

    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;

    std::ostringstream logMessage;
    logMessage << " -> Méthode : " << request.method
            << ", URI : " << request.uri
            << ", Version HTTP : " << request.httpVersion;
    LOG_INFO(logMessage.str());

    LOG_INFO(" -> Fin de l'analyse de la ligne de requête");
}

void HttpUtils_parseHeaders(std::istringstream& stream, HttpRequest& request)
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
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            std::string hexStr = str.substr(i + 1, 2);
            char decodedChar = static_cast<char>(std::strtol(hexStr.c_str(), NULL, 16));
            result.push_back(decodedChar);
            i += 2; // Skip the next two characters
        } else {
            result.push_back(str[i]);
        }
    }
    return result;
}

void HttpUtils_parseBody( HttpRequest& request)
{
     if (request.headers["Content-Type"] == "application/x-www-form-urlencoded") {
        std::istringstream dataStream(request.body);
        std::string pair;
        while (std::getline(dataStream, pair, '&')) {
            std::string::size_type pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                request.formData[key] = urlDecode(value);
            }
        }
    }
    else if (isMultipartFormData(request)) {
        std::string contentType = request.headers["Content-Type"];
        std::string boundary = getBoundary(contentType);

        if (!boundary.empty()) {
            std::vector<FilePart> files;
            parseMultipartFormData(request.body, boundary, files);
        }
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

bool isMultipartFormData(const HttpRequest& request)
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

std::string getBoundary(const std::string& contentType)
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

void parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files)
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

void saveFile(const std::string& content, const std::string& fileName)
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

FilePart extractFilePart(const std::string& part)
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
