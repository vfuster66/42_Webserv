#include "../includes/CgiRequestHandler.hpp"

extern char **environ;

HttpResponse CgiRequestHandler::handleRequest(const HttpRequest& request)
{
    HttpResponse response;

    std::string cgiOutput = executeCgiScript(request);

    if (cgiOutput.compare(0, 25, "Erreur interne du serveur") == 0)
    {
        response.statusCode = 500;
        response.body = cgiOutput;
    }
    else
    {
        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = cgiOutput;
        response.headers["Content-Type"] = "text/html";
    }

    return response;
}

void  CgiRequestHandler::setupCgiEnvironment(const HttpRequest& request)
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
    setenv("SCRIPT_NAME", "/cgi-bin/script.cgi", 1);
    setenv("PATH_INFO", request.uri.c_str(), 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("SERVER_SOFTWARE", "Webserv/1.0", 1);
    setenv("REDIRECT_STATUS", "200", 1);

}
std::string CgiRequestHandler::executeCgiScript(const HttpRequest& request)
{
    int cgiInput[2], cgiOutput[2];
    if (pipe(cgiInput) != 0 || pipe(cgiOutput) != 0)
    {
        return "Erreur interne du serveur: Impossible de créer les pipes.";
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        close(cgiInput[0]); close(cgiInput[1]);
        close(cgiOutput[0]); close(cgiOutput[1]);
        return "Erreur interne du serveur: Impossible de créer un processus enfant.";
    }

    if (pid == 0)
    {
        setupCgiEnvironment(request);

        if (dup2(cgiInput[0], STDIN_FILENO) == -1 || dup2(cgiOutput[1], STDOUT_FILENO) == -1)
        {
            exit(1); // Échec de dup2
        }
        close(cgiInput[1]);
        close(cgiOutput[0]);

        std::string interpreter = getCgiInterpreter(request.uri);
        if (interpreter.empty())
        {
            exit(2);
        }

        char* argv[] = { const_cast<char*>(interpreter.c_str()), NULL };
        if (execve(argv[0], argv, environ) == -1)
        {
            exit(3);
        }
    }
    else
    {

        close(cgiInput[0]);
        close(cgiOutput[1]);

        if (request.method == "POST")
        {
            const std::string& postData = request.body; // Assurez-vous que `HttpRequest` a un membre `body`
            if (!postData.empty())
            {
                ssize_t bytesWritten = write(cgiInput[1], postData.c_str(), postData.size());
                if (bytesWritten == -1 || bytesWritten < static_cast<ssize_t>(postData.size()))
                {
                    close(cgiInput[0]); close(cgiInput[1]);
                    close(cgiOutput[0]); close(cgiOutput[1]);
                    return "Erreur interne du serveur: Échec de l'écriture dans le pipe d'entrée du CGI.";
                }
            }
        }
        else if (request.method == "GET" || request.method == "DELETE")
        {
        }
        else
        {
            close(cgiInput[0]); close(cgiInput[1]);
            close(cgiOutput[0]); close(cgiOutput[1]);
            return "Erreur interne du serveur: Méthode HTTP non prise en charge.";
        }

        std::stringstream cgiResponse;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(cgiOutput[0], buffer, sizeof(buffer))) > 0)
        {
            cgiResponse.write(buffer, bytesRead);
        }
        close(cgiInput[1]);
        close(cgiOutput[0]);

        int status;
        waitpid(pid, &status, 0);

        if (bytesRead == -1)
        {
            // Gérer l'erreur de lecture
            return "Erreur interne du serveur: Échec de la lecture du pipe de sortie du CGI.";
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            // Gérer l'échec du script CGI
            return "Erreur interne du serveur: Le script CGI a échoué.";
        }
        return cgiResponse.str(); // Retourner la sortie du script CGI
    }
}

// Map des interpréteurs CGI par extension de fichier
std::map<std::string, std::string> cgiInterpreters =
{
    {".php", "/usr/bin/php-cgi"},
    {".py", "/usr/bin/python"}
    // Ajoutez d'autres interpréteurs CGI selon vos besoins
};

std::string getCgiInterpreter(const std::string& filePath)
{
    size_t dotPosition = filePath.find_last_of('.');
    if (dotPosition != std::string::npos)
    {
        std::string extension = filePath.substr(dotPosition);
        if (cgiInterpreters.count(extension) > 0)
        {
            return cgiInterpreters[extension];
        }
    }
    return ""; // Aucun interpréteur trouvé pour cette extension
}

std::string CgiRequestHandler::extractQueryString(const std::string& uri)
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

std::string CgiRequestHandler::extractScriptName(const std::string& uri)
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
std::string CgiRequestHandler::extractPathInfo(const std::string& uri)
{
    size_t pos = uri.find("?");
    std::string pathInfo = uri;

    if (pos != std::string::npos) {
        pathInfo = uri.substr(0, pos);
    }

    return pathInfo;
}

std::string CgiRequestHandler::convertPathInfoToFilePath(const std::string& pathInfo)
{
    const std::string cgiBinPrefix = "/cgi-bin/";
    const std::string cgiBinDirectory = "/var/www/cgi-bin/";

    if (pathInfo.find(cgiBinPrefix) == 0)
    {
        std::string filePath = cgiBinDirectory + pathInfo.substr(cgiBinPrefix.length());
        return filePath;
    }
    return pathInfo;
}
