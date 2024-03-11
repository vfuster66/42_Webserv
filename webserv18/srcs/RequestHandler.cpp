#include "../includes/RequestHandler.hpp"

RequestHandler::RequestHandler() : current_port(-1)
{
}

void RequestHandler::setServerConfigs(const std::vector<ServerConfig>& configs)
{
    serverConfigs = configs;
}

void RequestHandler::setServerPort(int port)
{
    current_port = port;
    std::ostringstream logMessage;
    logMessage << "Port du serveur mis à jour : " << port;
    LOG_INFO(logMessage.str());
}

int RequestHandler::extractPortFromHostHeader(const std::string& hostHeader)
{
    size_t colonPos = hostHeader.find_last_of(':');
    if (colonPos != std::string::npos)
    {
        std::istringstream portStrStream(hostHeader.substr(colonPos + 1));
        int port;
        portStrStream >> port;
        std::stringstream ss;
        ss << port;
        LOG_INFO("Port extracted from host header: " + ss.str());
        return port;
    }
    LOG_INFO("Default port 80 used");
    return 80;
}

ServerConfig RequestHandler::getServerConfigForPort(int port)
{
    for (std::vector<ServerConfig>::iterator it = serverConfigs.begin(); it != serverConfigs.end(); ++it)
    {
        if (it->port == port)
        {
            std::stringstream ss;
            ss << port;
            std::string logMessage = "Server config found for port: " + ss.str();
            LOG_INFO(logMessage.c_str());
            return *it;
        }
    }

    std::stringstream ss;
    ss << port;
    std::string logErrorMessage = "Server config not found for port: " + ss.str();
    LOG_ERROR(logErrorMessage.c_str());
    
    throw std::runtime_error("Server config not found for port: " + ss.str());
}


bool RequestHandler::isValidRequest(const HttpRequest& request)
{
    if(request.method != "GET" && request.method != "POST" && request.method != "DELETE")
    {
        LOG_ERROR("Méthode HTTP non supportée: " + request.method);
        return false;
    }
    LOG_INFO("Requête valide avec la méthode: " + request.method);
    return true;
}


HttpResponse RequestHandler::handleRequest(const HttpRequest& request)
{

    LOG_INFO("handling request for URI: " + request.uri);

    if (!isValidRequest(request)) {
        // Log invalid request
        LOG_ERROR("Invalid request received");
        HttpResponse response;
        response.httpVersion = "HTTP/1.1";
        response.statusCode = 400;
        response.statusMessage = "Bad Request";
        response.body = loadErrorPage(400);
        return response;
    }

    if (isCgiRequest(request)) {
        // Log CGI request
        LOG_INFO("CGI request detected");
        return handleCgiRequest(request);
    }
    else if (request.method == "GET")
    {
        // Log GET request
        if (shouldBeRedirected(request.uri))
        {
            return handleGetRequestWithRedirection(request); // Gère la redirection
        } else {
            return handleGetRequest(request); // Votre traitement GET classique existant
        }
    }
    else if (request.method == "POST") {
        // Log POST request
        LOG_INFO("POST request received");
        return handlePostRequest(request);
    }
    else if (request.method == "DELETE") {
        // Log DELETE request
        LOG_INFO("DELETE request received");
        return handleDeleteRequest(request);
    }
    else {
        LOG_ERROR("Unsupported method: " + request.method);
        HttpResponse response;
        response.httpVersion = "HTTP/1.1";
        response.statusCode = 405;
        response.statusMessage = "Method Not Allowed";
        response.headers.insert(std::make_pair(std::string("Allow"), std::string("GET, POST, DELETE")));
        response.body = loadErrorPage(405);
        response.headers.insert(std::make_pair(std::string("Content-Type"), std::string("text/html")));
        std::ostringstream contentLengthStream;
        contentLengthStream << response.body.length();
        response.headers.insert(std::make_pair(std::string("Content-Length"), contentLengthStream.str()));
        return response;
    }
}

std::string RequestHandler::getAbsolutePath(const std::string& uri, int port)
{
    const ServerConfig& config = getServerConfigForPort(port); // Utilisez le port pour obtenir la config
    std::string basePath = config.root; // Utilisez la racine de la config
    std::string fullPath = basePath + uri;

    // Normalisation du chemin
    std::string normalizedPath = normalizePath(fullPath);

    return normalizedPath;
}

std::string RequestHandler::normalizePath(const std::string& path)
{
    std::vector<std::string> pathParts;
    std::stringstream pathStream(path);
    std::string part;

    while (std::getline(pathStream, part, '/'))
    {
        if (part == "..")
        {
            // Go up one directory, if possible
            if (!pathParts.empty()) pathParts.pop_back();
        }
        else if (!part.empty() && part != ".")
        {
            // Ignore empty parts and current directory marker "."
            pathParts.push_back(part);
        }
    }

    std::string normalizedPath;
    for (size_t i = 0; i < pathParts.size(); ++i) {
        normalizedPath += "/" + pathParts[i];
    }

    return normalizedPath;
}

bool RequestHandler::isDirectory(const std::string& path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return false;
    return S_ISDIR(statbuf.st_mode);
}

/*std::string RequestHandler::generateDirectoryListingHtml(const std::string& directoryPath)
{
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        // Gérer l'erreur d'ouverture
        return "<html><body><p>Erreur lors de l'ouverture du répertoire.</p></body></html>";
    }

    struct dirent* entry;
    std::ostringstream htmlStream;
    htmlStream << "<html><body><ul>";

    while ((entry = readdir(dir)) != NULL) {
        htmlStream << "<li>" << entry->d_name << "</li>";
    }

    htmlStream << "</ul></body></html>";
    closedir(dir);

    return htmlStream.str();
}*/
std::string RequestHandler::generateDirectoryListingHtml(const std::string& directoryPath)
{
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        // Gérer l'erreur d'ouverture de manière plus descriptive
        return "<html><body><p>Impossible d'ouvrir le répertoire : " + directoryPath + "</p></body></html>";
    }

    struct dirent* entry;
    std::ostringstream htmlStream;
    htmlStream << "<html><head><style>li.folder:before { content: '📁 '; } li.file:before { content: '📄 '; }</style></head><body><ul>";

    while ((entry = readdir(dir)) != NULL) {
        std::string entryName(entry->d_name);
        // Ignorer les entrées '.' et '..'
        if (entryName == "." || entryName == "..") continue;

        std::string fullPath = directoryPath + "/" + entryName;
        struct stat entryStat;
        stat(fullPath.c_str(), &entryStat);

        if (S_ISDIR(entryStat.st_mode)) {
            // C'est un dossier
            htmlStream << "<li class='folder'><a href='" << entryName << "/'>" << entryName << "</a></li>";
        } else {
            // C'est un fichier
            htmlStream << "<li class='file'><a href='" << entryName << "'>" << entryName << "</a></li>";
        }
    }

    htmlStream << "</ul></body></html>";
    closedir(dir);

    return htmlStream.str();
}


bool RequestHandler::shouldBeRedirected(const std::string& uri)
{
    // Déterminez ici si l'URI donnée doit être redirigée
    // Par exemple :
    return uri == "/ancienne_page";
}

std::string RequestHandler::getScriptPathFromUri(const std::string& uri)
{
    LOG_INFO("getScriptPathFromUri -> URI received: " + uri);

    std::string cgiBasePath = "/home/ecullier/webserv15/www/cgi-bin/";
    if (uri.find(cgiBasePath) != std::string::npos)
    {
        LOG_INFO("Full CGI script path: " + uri);
        return uri;
    }
    else
    {
        LOG_ERROR("Error: URI path does not contain the expected base path for CGI scripts.");
    }
    return "";
}



HttpResponse RequestHandler::handleCgiRequest(const HttpRequest& request)
{
    LOG_INFO("Début de la gestion de la requête CGI avec URI: " + request.uri);

    try
    {
        int port = extractPortFromHostHeader(request.headers.find("Host")->second);
        const ServerConfig& serverConfig = getServerConfigForPort(port);

        std::string scriptPath = getScriptPathFromUri(request.uri);
        if (scriptPath.empty())
        {
            return generateNotFoundResponse();
        }

        CgiHandler cgiHandler(scriptPath, request);
        LOG_INFO("CgiHandler construit avec scriptPath: " + scriptPath);

        HttpResponse response = cgiHandler.executeScript();
        LOG_INFO("Script CGI exécuté, préparation de la réponse HTTP");

        return response;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Exception capturée lors de la gestion de la requête CGI: " + std::string(e.what()));
        return generateInternalServerErrorResponse();
    }
}

bool RequestHandler::isCgiRequest(const HttpRequest& request)
{
    LOG_INFO("isCgiRequest called with URI: " + request.uri);

    std::string uri = request.uri;
    size_t lastDotPos = uri.find_last_of(".");
    if (lastDotPos != std::string::npos)
    {
        std::string extension = uri.substr(lastDotPos);
        LOG_INFO("Extension extracted: " + extension);
        if (extension == ".cgi" || extension == ".pl" || extension == ".php" || extension == ".py")
        {
            LOG_INFO("Request identified as CGI.");
            return true;
        }
    }

    LOG_INFO("Request not identified as CGI.");
    return false;
}

/*HttpResponse RequestHandler::handleGetRequest(const HttpRequest& request)
{
    HttpResponse response;
    // Extrait le port depuis l'en-tête 'Host' de la requête
    int port = extractPortFromHostHeader(request.headers.find("Host")->second);

    try
    {
        // Obtient la configuration du serveur pour le port spécifié
        const ServerConfig& serverConfig = getServerConfigForPort(port);

        // Construit le chemin complet vers la ressource demandée
        std::string fullPath = serverConfig.root + request.uri;
        // Assurez-vous que le chemin ne se termine pas par un slash
        if (fullPath.back() == '/') {
            fullPath.pop_back();
        }

        struct stat pathStat;
        // Vérifie si le chemin existe et est un fichier ou un répertoire
        if (stat(fullPath.c_str(), &pathStat) == 0)
        {
            if (S_ISREG(pathStat.st_mode))
            {
                // Si c'est un fichier, lit et renvoie son contenu
                std::ifstream file(fullPath, std::ios::binary);
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();

                response.httpVersion = "HTTP/1.1";
                response.statusCode = 200;
                response.statusMessage = "OK";
                response.body = content;
                response.headers["Content-Type"] = determineMimeType(fullPath) + "; charset=utf-8";
            }
            else if (S_ISDIR(pathStat.st_mode))
            {
                // Si c'est un répertoire, essaie de trouver un fichier index.html
                std::string indexPath = fullPath + "/index.html";
                std::ifstream indexFile(indexPath, std::ios::binary);
                if (indexFile)
                {
                    // Si index.html est trouvé, lit et renvoie son contenu
                    std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
                    indexFile.close();

                    response.httpVersion = "HTTP/1.1";
                    response.statusCode = 200;
                    response.statusMessage = "OK";
                    response.body = content;
                    response.headers["Content-Type"] = "text/html; charset=utf-8";
                }
                else if (serverConfig.directory_listing)
                {
                    // Si index.html n'est pas trouvé mais que le listing de répertoire est autorisé, génère et renvoie le listing
                    response.body = generateDirectoryListingHtml(fullPath);
                    response.statusCode = 200; // OK
                    response.headers["Content-Type"] = "text/html; charset=utf-8";
                }
                else
                {
                    // Si le listing de répertoire n'est pas autorisé, renvoie une erreur 404
                    response = generateNotFoundResponse();
                }
            }
        }
        else
        {
            // Si le chemin n'existe pas, renvoie une erreur 404
            response = generateNotFoundResponse();
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Erreur: " << e.what() << std::endl;
        // En cas d'erreur, génère une réponse d'erreur interne du serveur
        return generateInternalServerErrorResponse();
    }

    // Définit la longueur du contenu dans l'en-tête de la réponse
    std::ostringstream contentLengthStream;
    contentLengthStream << response.body.size();
    response.headers["Content-Length"] = contentLengthStream.str();

    return response;
}*/
HttpResponse RequestHandler::handleGetRequest(const HttpRequest& request)
{
    HttpResponse response;
    int port = extractPortFromHostHeader(request.headers.find("Host")->second);
    
    try
    {
        const ServerConfig& serverConfig = getServerConfigForPort(port);
        std::string fullPath = serverConfig.root + request.uri;

        if (fullPath.back() == '/') {
            fullPath.erase(fullPath.size() - 1); // Supprime le slash final si présent
        }

        struct stat pathStat;
        if (stat(fullPath.c_str(), &pathStat) == 0) // Vérifie si le chemin existe
        {
            if (S_ISREG(pathStat.st_mode)) // Si c'est un fichier
            {
                std::ifstream file(fullPath, std::ios::binary);
                response.body = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                response.httpVersion = "HTTP/1.1";
                response.statusCode = 200; // OK
                response.headers["Content-Type"] = determineMimeType(fullPath) + "; charset=utf-8";
            }
            else if (S_ISDIR(pathStat.st_mode)) // Si c'est un répertoire
            {
                std::string indexPath = fullPath + "/" + (serverConfig.index.empty() ? "index.html" : serverConfig.index.front());
                std::ifstream indexFile(indexPath, std::ios::binary);
                if (indexFile) // Si le fichier index spécifié existe
                {
                    response.body = std::string((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
                    response.httpVersion = "HTTP/1.1";
                    response.statusCode = 200; // OK
                    response.headers["Content-Type"] = "text/html; charset=utf-8";
                }
                else if (serverConfig.directory_listing) // Si le listing du répertoire est autorisé
                {
                    response.body = generateDirectoryListingHtml(fullPath);
                    response.httpVersion = "HTTP/1.1";
                    response.statusCode = 200; // OK
                    response.headers["Content-Type"] = "text/html; charset=utf-8";
                }
                else // Si le listing du répertoire n'est pas autorisé
                {
                    response = generateNotFoundResponse();
                }
            }
        }
        else // Si le chemin n'existe pas
        {
            response = generateNotFoundResponse();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur: " << e.what() << std::endl;
        response = generateInternalServerErrorResponse();
    }

    // Définit la longueur du contenu dans l'en-tête de la réponse
    response.headers["Content-Length"] = std::to_string(response.body.size());

    return response;
}



HttpResponse RequestHandler::generateInternalServerErrorResponse()
{
    HttpResponse response;
    response.httpVersion = "HTTP/1.1";
    response.statusCode = 500;
    response.statusMessage = "Internal Server Error";
    response.body = loadErrorPage(500);
    return response;
}

HttpResponse RequestHandler::generateNotFoundResponse()
{
    HttpResponse response;
    response.httpVersion = "HTTP/1.1";
    response.statusCode = 404;
    response.statusMessage = "Not Found";
    
    response.body = loadErrorPage(404);

    std::ostringstream contentLengthStream;
    contentLengthStream << response.body.size();
    response.headers["Content-Length"] = contentLengthStream.str();

    response.headers["Content-Type"] = "text/html";

    return response;
}



HttpResponse RequestHandler::handleDeleteRequest(const HttpRequest& request)
{
    HttpResponse response;

    std::string filePath = "www" + request.uri;

    std::ifstream file(filePath.c_str());
    if (file)
    {
        file.close();

        if (std::remove(filePath.c_str()) == 0)
        {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 204;
            response.statusMessage = "No Content";
            
        }
        else
        {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 500;
            response.statusMessage = "Internal Server Error";
            response.body = loadErrorPage(500);
            response.headers["Content-Type"] = "text/html";

        }
    }
    else
    {
        response.httpVersion = "HTTP/1.1";
        response.statusCode = 404;
        response.statusMessage = "Not Found";
        response.body = loadErrorPage(404);
    }

    if (!response.body.empty())
    {
        std::ostringstream oss;
        oss << response.body.length();
        response.headers["Content-Length"] = oss.str();
    }

    return response;
}


HttpResponse RequestHandler::handleGetRequestWithRedirection(const HttpRequest& request)
{
    HttpResponse response;

    // Vérifier si l'URL demandée nécessite une redirection
    if (request.uri == "/ancienne_page") {
        response.httpVersion = "HTTP/1.1";
        response.statusCode = 301;  // Utiliser 302 pour une redirection temporaire
        response.statusMessage = "Moved Permanently";
        response.headers["Location"] = "http://votre_serveur:3500/nouvelle_page";  // URL de destination
        // Aucun corps de réponse n'est nécessaire pour une redirection
    }else {
        // Si aucune redirection n'est nécessaire, traiter la requête GET normalement
        response = handleGetRequest(request);
    }

    return response;
}

HttpResponse RequestHandler::handlePostRequest(const HttpRequest& request)
{
    HttpResponse response;

    if (isMultipartFormData(request))
    {
        std::string contentType = request.headers.find("Content-Type")->second;
        std::string boundary = getBoundary(contentType);

        if (!boundary.empty())
        {
            std::vector<FilePart> files;
            parseMultipartFormData(request.body, boundary, files);

            for (size_t i = 0; i < files.size(); ++i)
            {
                saveFile(files[i].fileContent, files[i].fileName);
            }

            response.httpVersion = "HTTP/1.1";
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = "<html><body><h1>File(s) Uploaded Successfully</h1></body></html>";
            response.headers["Content-Type"] = "text/html";
        }
        else
        {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 400;
            response.statusMessage = "Bad Request";
            response.body = loadErrorPage(400);
            response.headers["Content-Type"] = "text/html";
        }
    }
    else
    {
        std::map<std::string, std::string> formData = parseFormData(request.body);

        bool success = performAction(formData);

        if (success)
        {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 200;
            response.statusMessage = "OK";
            response.body = "<html><body><h1>Success</h1><p>Your data was processed successfully.</p></body></html>";
            response.headers["Content-Type"] = "text/html";
        }
        else
        {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 400;
            response.statusMessage = "Bad Request";
            response.body = loadErrorPage(400);
            response.headers["Content-Type"] = "text/html";
        }

        std::ostringstream oss;
        oss << response.body.length();
        response.headers["Content-Length"] = oss.str();
    }

    return response;
}

std::map<std::string, std::string> RequestHandler::parseFormData(const std::string& body)
{
    std::map<std::string, std::string> formData;
    std::istringstream dataStream(body);
    std::string pair;

    while (std::getline(dataStream, pair, '&'))
    {
        std::string::size_type pos = pair.find('=');
        if (pos != std::string::npos)
        {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            for (std::string::iterator it = value.begin(); it != value.end(); ++it)
            {
                if (*it == '+') *it = ' ';
            }

            formData[key] = urlDecode(value);
        }
    }
    return formData;
}


bool RequestHandler::performAction(const std::map<std::string, std::string>& formData)
{
    std::ofstream outFile("form_data.txt", std::ios::app);
    if (!outFile.is_open())
    {
        std::cerr << "Erreur lors de l'ouverture du fichier pour écriture." << std::endl;
        return false;
    }

    for (std::map<std::string, std::string>::const_iterator it = formData.begin(); it != formData.end(); ++it)
    {
        outFile << it->first << ": " << it->second << std::endl;
    }

    outFile.close();
    return true;
}

std::string urlDecode(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.size())
        {
            std::string hexStr = str.substr(i + 1, 2);
            char decodedChar = static_cast<char>(std::strtol(hexStr.c_str(), NULL, 16));
            result.push_back(decodedChar);
            i += 2;
        }
        else
        {
            result.push_back(str[i]);
        }
    }
    return result;
}


//parsing
void RequestHandler::parseRequestLine(const std::string& line, HttpRequest& request)
{
    LOG_INFO("Début de l'analyse de la ligne de requête");

    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;

    std::ostringstream logMessage;
    logMessage << "Méthode : " << request.method
            << ", URI : " << request.uri
            << ", Version HTTP : " << request.httpVersion;
    LOG_INFO(logMessage.str());

    LOG_INFO("pFin de l'analyse de la ligne de requête");
}

void RequestHandler::parseHeaders(std::istringstream& stream, HttpRequest& request)
{
    LOG_INFO("Début de l'analyse des en-têtes HTTP");

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
            logMessage << "En-tête trouvé : " << key << " : " << value;
            LOG_INFO(logMessage.str());
        }
    }

    LOG_INFO("Fin de l'analyse des en-têtes HTTP");
}

void RequestHandler::parseBody(HttpRequest& request)
{
    if (request.headers["Content-Type"] == "application/x-www-form-urlencoded")
    {
        std::istringstream dataStream(request.body);
        std::string pair;
        while (std::getline(dataStream, pair, '&'))
        {
            std::string::size_type pos = pair.find('=');
            if (pos != std::string::npos)
            {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                request.formData[key] = urlDecode(value);
            }
        }
    }
    else if (isMultipartFormData(request))
    {
        std::string contentType = request.headers["Content-Type"];
        std::string boundary = getBoundary(contentType);
        
        if (!boundary.empty())
        {
            std::vector<FilePart> files;
            parseMultipartFormData(request.body, boundary, files);
        }
    }
}

HttpRequest RequestHandler::parseRequest(const std::string& requestText)
{
    LOG_INFO("Début du parsing de la requête");

    HttpRequest request;
    std::istringstream stream(requestText);
    std::string requestLine;
    std::getline(stream, requestLine);

    LOG_INFO("Parsing de la ligne de requête");
    parseRequestLine(requestLine, request);

    LOG_INFO("Parsing des en-têtes");
    parseHeaders(stream, request);

    LOG_INFO("Parsing du corps");
    parseBody(request);

    LOG_INFO("Parsing de la requête terminé");

    return request;
}

// Extensions
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

    LOG_INFO("Type MIME déterminé pour " + filePath + " : " + mimeType);
    
    return mimeType;
}

// erreurs
std::string RequestHandler::loadErrorPage(int statusCode)
{
    std::ostringstream filePathStream;
    filePathStream << "www/errors/" << statusCode << ".html";
    std::string filePath = filePathStream.str();

    LOG_INFO("Tentative de chargement de la page d'erreur : " + filePath);

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        LOG_INFO("Page d'erreur chargée avec succès : " + filePath);
        return content;
    }
    else
    {
        LOG_ERROR("Échec du chargement de la page d'erreur, fichier non trouvé : " + filePath);
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
        LOG_INFO("Content-Type trouvé : " + contentType);
    }
    else
    {
        LOG_WARNING("Aucun Content-Type trouvé dans les en-têtes de la requête.");
    }

    bool isMultipart = contentType.find("multipart/form-data") != std::string::npos;

    if (isMultipart)
    {
        LOG_INFO("La requête est de type multipart/form-data.");
    }
    else
    {
        LOG_INFO("La requête n'est pas de type multipart/form-data.");
    }

    return isMultipart;
}

std::string RequestHandler::getBoundary(const std::string& contentType)
{
    LOG_INFO("Début de getBoundary avec contentType: " + contentType);

    std::string::size_type pos = contentType.find("boundary=");
    if (pos != std::string::npos)
    {
        std::string boundary = contentType.substr(pos + 9);
        LOG_INFO("Boundary trouvé: " + boundary);
        return boundary;
    }

    LOG_WARNING("Aucun boundary trouvé dans contentType");
    return "";
}

void RequestHandler::parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files)
{
    LOG_INFO("Début de parseMultipartFormData");

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
            LOG_INFO("Fichier extrait: " + filePart.fileName);
            files.push_back(filePart);
        }

        pos = endPos + delimiter.length();
    }

    LOG_INFO("Fin de parseMultipartFormData");
}

void RequestHandler::saveFile(const std::string& content, const std::string& fileName)
{
    std::string filePath = "/home/ecullier/webserv15/uploads/" + fileName;

    std::ofstream file(filePath.c_str(), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write(content.c_str(), content.size());
        file.close();
        LOG_INFO("Fichier sauvegardé avec succès : " + filePath);
    }
    else
    {
        LOG_ERROR("Erreur lors de l'ouverture du fichier pour écriture : " + filePath);
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
            LOG_INFO("Début du contenu du fichier détecté.");
            continue;
        }

        if (!contentStart)
        {
            if (line.find("Content-Disposition:") != std::string::npos)
            {
                contentDisposition = line;
                LOG_INFO("Content-Disposition trouvé : " + line);
                std::size_t filenamePos = line.find("filename=\"");
                if (filenamePos != std::string::npos)
                {
                    filenamePos += 10;
                    std::size_t filenameEnd = line.find("\"", filenamePos);
                    if (filenameEnd != std::string::npos)
                    {
                        filePart.fileName = line.substr(filenamePos, filenameEnd - filenamePos);
                        LOG_INFO("Nom du fichier extrait : " + filePart.fileName);
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

    LOG_INFO("Extraction du fichier terminée. Nom du fichier : " + filePart.fileName);
    return filePart;
}
