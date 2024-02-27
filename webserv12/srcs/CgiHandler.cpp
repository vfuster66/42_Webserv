#include "../includes/CgiHandler.hpp"

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

CgiHandler::CgiHandler(const CgiHandler& other)
    : scriptPath(other.scriptPath), cgiEnv(other.cgiEnv)
{
    // Logique de copie supplémentaire si nécessaire
}

CgiHandler& CgiHandler::operator=(const CgiHandler& other)
{
    if (this != &other)
    {
        scriptPath = other.scriptPath;
        cgiEnv = other.cgiEnv;
        // Logique d'assignation supplémentaire si nécessaire
    }
    return *this;
}

HttpResponse CgiHandler::executeCgi(const HttpRequest& request) {
    return handleCgiRequest(request);
}

HttpResponse CgiHandler::handleCgiRequest(const HttpRequest& request)
{
    HttpResponse response;
    try {
        setupCgiEnvironment(request); // Configure les variables d'environnement pour le script CGI

        std::string cgiOutput = executeCgiScript(request); // Exécute le script CGI et capture la sortie
        response.statusCode = 200; // Supposons que l'exécution est toujours réussie pour cet exemple
        response.body = cgiOutput; // Utilise la sortie du script CGI comme corps de la réponse HTTP
        LOG_INFO("CGI script executed successfully");
    } catch (const std::exception& e) {
        response.statusCode = 500;
        response.body = "Internal Server Error"; // Message d'erreur générique
        LOG_ERROR("Failed to execute CGI script: " + std::string(e.what()));
    }

    return response;
}

// Exemple de fonction pour configurer l'environnement CGI
void CgiHandler::setupCgiEnvironment(const HttpRequest& request)
{
    std::string pathInfo = extractPathInfo(request.uri);
    setenv("PATH_INFO", pathInfo.c_str(), 1);

    setenv("REQUEST_METHOD", request.method.c_str(), 1);

    std::string queryString = request.uri.find('?') != std::string::npos ? request.uri.substr(request.uri.find('?') + 1) : "";
    setenv("QUERY_STRING", queryString.c_str(), 1);

    std::string pathTranslated = convertPathInfoToFilePath(pathInfo);
    setenv("PATH_TRANSLATED", pathTranslated.c_str(), 1);

    std::string contentType = request.headers.count("Content-Type") > 0 ? request.headers.at("Content-Type") : "";
    if (!contentType.empty())
    {
        setenv("CONTENT_TYPE", contentType.c_str(), 1);
    }
    std::string contentLength = request.headers.count("Content-Length") > 0 ? request.headers.at("Content-Length") : "";
    if (!contentLength.empty())
    {
        setenv("CONTENT_LENGTH", contentLength.c_str(), 1);
    }
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("SERVER_SOFTWARE", "VotreServeur/1.0", 1);

    // Variables d'environnement basées sur la requête HTTP
    setenv("REQUEST_METHOD", request.method.c_str(), 1);
    setenv("SCRIPT_NAME", "/chemin/vers/le/script.cgi", 1); // À ajuster selon votre configuration
    setenv("QUERY_STRING", extractQueryString(request.uri).c_str(), 1);
    setenv("PATH_INFO", extractPathInfo(request.uri).c_str(), 1); // Vous devrez implémenter extractPathInfo
    setenv("CONTENT_TYPE", request.headers.count("Content-Type") ? request.headers.at("Content-Type").c_str() : "", 1);
    setenv("CONTENT_LENGTH", request.headers.count("Content-Length") ? request.headers.at("Content-Length").c_str() : "", 1);

    // Ajoutez ici d'autres variables d'environnement si nécessaire
    LOG_INFO("CGI environment setup completed");
}

std::string CgiHandler::executeCgiScript(const HttpRequest& request)
{
    int cgiInput[2], cgiOutput[2];
    if (pipe(cgiInput) != 0 || pipe(cgiOutput) != 0) {
        LOG_ERROR("Erreur interne du serveur: Impossible de créer les pipes.");
        return "Erreur interne du serveur: Impossible de créer les pipes.";
    }

    pid_t pid = fork();
    if (pid < 0) {
        LOG_ERROR("Erreur interne du serveur: Impossible de créer un processus enfant.");
        close(cgiInput[0]); close(cgiInput[1]);
        close(cgiOutput[0]); close(cgiOutput[1]);
        return "Erreur interne du serveur: Impossible de créer un processus enfant.";
    }

    if (pid == 0) { // Processus enfant
        setupCgiEnvironment(request);

        if (dup2(cgiInput[0], STDIN_FILENO) == -1 || dup2(cgiOutput[1], STDOUT_FILENO) == -1) {
            LOG_ERROR("Échec de dup2 lors de la configuration des descripteurs de fichier pour le CGI.");
            exit(1);
        }
        close(cgiInput[1]);
        close(cgiOutput[0]);

        std::string interpreter = getCgiInterpreter(request.uri);
        if (interpreter.empty()) {
            LOG_ERROR("Aucun interpréteur trouvé pour l'extension du fichier CGI.");
            exit(2);
        }

        char* argv[] = { const_cast<char*>(interpreter.c_str()), NULL };
        if (execve(argv[0], argv, environ) == -1)
	{
            LOG_ERROR("Échec de l'exécution du script CGI via execve.");
            exit(3);
        }
    } else { // Processus parent
        close(cgiInput[0]);
        close(cgiOutput[1]);

        if (request.method == "POST") {
            const std::string& postData = request.body;
            ssize_t bytesWritten = write(cgiInput[1], postData.c_str(), postData.size());
            if (bytesWritten == -1 || bytesWritten < static_cast<ssize_t>(postData.size())) {
                LOG_ERROR("Échec de l'écriture dans le pipe d'entrée du CGI pour les données POST.");
                close(cgiInput[1]); close(cgiOutput[0]);
                return "Erreur interne du serveur: Échec de l'écriture dans le pipe d'entrée du CGI.";
            }
        }

        std::stringstream cgiResponse;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(cgiOutput[0], buffer, sizeof(buffer))) > 0) {
            cgiResponse.write(buffer, bytesRead);
        }
        close(cgiInput[1]);
        close(cgiOutput[0]);

        int status;
        waitpid(pid, &status, 0);

        if (bytesRead == -1) {
            LOG_ERROR("Échec de la lecture du pipe de sortie du CGI.");
            return "Erreur interne du serveur: Échec de la lecture du pipe de sortie du CGI.";
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            LOG_ERROR("Le script CGI a échoué.");
            return "Erreur interne du serveur: Le script CGI a échoué.";
        }
        return cgiResponse.str();
    }
    return "";
}


std::string CgiHandler::extractQueryString(const std::string& uri)
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

std::map<std::string, std::string> CgiHandler::cgiInterpreters ;


void CgiHandler::initCgiInterpreters() {
    // Fusionnez ici les interpréteurs de CgiHandler.cpp et CgiHandler.cpp
    cgiInterpreters[".php"] = "/opt/homebrew/bin/php-cgi"; // De CgiHandler.cpp
    cgiInterpreters[".py"] = "/Library/Frameworks/Python.framework/Versions/3.12/bin/python3"; // De CgiHandler.cpp
    // Ajoutez d'autres interpréteurs au besoin
}

std::string CgiHandler::getCgiInterpreter(const std::string& filePath) {
    size_t dotPosition = filePath.find_last_of('.');
    if (dotPosition != std::string::npos) {
        std::string extension = filePath.substr(dotPosition);
        if (cgiInterpreters.count(extension) > 0) {
            return cgiInterpreters[extension];
        }
    }
    return ""; // Aucun interpréteur trouvé pour cette extension
}


std::string CgiHandler::extractScriptName(const std::string& uri)
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
std::string CgiHandler::extractPathInfo(const std::string& uri)
{
    size_t pos = uri.find("?");
    std::string pathInfo = uri;

    if (pos != std::string::npos) {
        pathInfo = uri.substr(0, pos);
    }

    return pathInfo;
}

std::string CgiHandler::convertPathInfoToFilePath(const std::string& pathInfo)
{
    const std::string cgiBinPrefix = "/cgi-bin/";
    const std::string cgiBinDirectory = "/webserv11/www/cgi-bin/";

    if (pathInfo.find(cgiBinPrefix) == 0)
    {
        std::string filePath = cgiBinDirectory + pathInfo.substr(cgiBinPrefix.length());
        return filePath;
    }
    return pathInfo;
}