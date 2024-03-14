#include "../includes/RequestHandler.hpp"
#include "../includes/CgiHandler.hpp"

CgiHandler::CgiHandler(const std::string& scriptPath, const HttpRequest& request, RequestHandler& handler)
: scriptPath(scriptPath), request(request), handler(handler)
{
    LOG_INFO("Initialisation de CgiHandler avec le script: " + scriptPath);
    setupEnvironment();
}

CgiHandler::~CgiHandler()
{
}

void CgiHandler::setupEnvironment()
{
    LOG_INFO("Configuration de l'environnement CGI pour le script: " + scriptPath);

    cgiEnvironment["REQUEST_METHOD"] = request.method;
    cgiEnvironment["QUERY_STRING"] = request.queryString;
    cgiEnvironment["CONTENT_TYPE"] = request.getHeader("Content-Type");
    cgiEnvironment["CONTENT_LENGTH"] = request.getHeader("Content-Length");
    cgiEnvironment["SCRIPT_FILENAME"] = scriptPath;
}

HttpResponse CgiHandler::executeScript()
{
    LOG_INFO("Début de l'exécution du script CGI.");

    int outputPipefd[2];
    if (pipe(outputPipefd) != 0)
    {
        LOG_ERROR("Erreur lors de la création du pipe de sortie.");
        return HttpResponse();
    }
    LOG_INFO("Pipe de sortie créé avec succès.");

    int inputPipefd[2];
    if (pipe(inputPipefd) != 0)
    {
        LOG_ERROR("Erreur lors de la création du pipe d'entrée.");
        close(outputPipefd[0]);
        close(outputPipefd[1]);
        return HttpResponse();
    }
    LOG_INFO("Pipe d'entrée créé avec succès.");

    pid_t pid = fork();
    if (pid == -1)
    {
        LOG_ERROR("Erreur lors de l'exécution de fork().");
        close(outputPipefd[0]);
        close(outputPipefd[1]);
        close(inputPipefd[0]);
        close(inputPipefd[1]);
        return HttpResponse();
    }

    if (pid == 0)
    {
        close(outputPipefd[0]);
        dup2(outputPipefd[1], STDOUT_FILENO);
        dup2(outputPipefd[1], STDERR_FILENO);
        close(outputPipefd[1]);

        close(inputPipefd[1]);
        dup2(inputPipefd[0], STDIN_FILENO);
        close(inputPipefd[0]);

        char* argv[] = {NULL};
        char** envp = createEnvp(cgiEnvironment);
        execve(scriptPath.c_str(), argv, envp);
        LOG_ERROR("Échec de execve.");
        freeEnvp(envp);
        exit(EXIT_FAILURE);
    }
    else
    {
        close(outputPipefd[1]);
        close(inputPipefd[0]);

        std::ostringstream ss;
        if (!request.body.empty())
        {
            ssize_t written = write(inputPipefd[1], request.body.c_str(), request.body.size());
            if (written < 0)
            {
                LOG_ERROR("Erreur lors de l'écriture dans le pipe d'entrée.");
            }
            else
            {
                ss << "Écriture de " << written << " octets dans le pipe d'entrée.";
                LOG_INFO(ss.str());
                ss.str("");
            }
        }
        close(inputPipefd[1]);

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(outputPipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }
        close(outputPipefd[0]);
        ss << "Lecture de la sortie du script CGI terminée. Taille des données : " << output.size() << " octets.";
        LOG_INFO(ss.str());

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
            // Ici, vous savez que execve a échoué
            HttpResponse errorResponse;
            errorResponse.httpVersion = "HTTP/1.1";
            errorResponse.statusCode = 500;
            errorResponse.statusMessage = "Internal Server Error";
            // Vous devez avoir une fonction loadErrorPage qui charge une page d'erreur depuis un fichier
            errorResponse.body = handler.loadErrorPage(500);
            errorResponse.headers["Content-Type"] = "text/html";
            std::ostringstream contentLengthStream;
            contentLengthStream << errorResponse.body.size();
            errorResponse.headers["Content-Length"] = contentLengthStream.str();
            return errorResponse;
        }

        return parseCgiOutput(output);
    }
}

char** CgiHandler::createEnvp(const std::map<std::string, std::string>& envMap)
{
    char** envp = new char*[envMap.size() + 1];
    int i = 0;

    for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it)
    {
        std::string env = it->first + "=" + it->second;
        envp[i] = strdup(env.c_str());
        LOG_INFO("Variable d'environnement ajoutée: " + env);
        i++;
    }

    envp[i] = NULL;
    return envp;
}

void CgiHandler::freeEnvp(char** envp)
{
    for (int i = 0; envp[i] != NULL; i++)
    {
        LOG_INFO(std::string("Libération de la variable d'environnement: ") + envp[i]);
        free(envp[i]);
    }

    delete[] envp;
    LOG_INFO("fLibération complète de envp.");
}

HttpResponse CgiHandler::parseCgiOutput(const std::string& cgiOutput)
{
    HttpResponse response;

    std::istringstream stream(cgiOutput);
    std::string line;
    bool headerSection = true;

    while (getline(stream, line))
    {
        if (headerSection && line.empty())
        {
            headerSection = false;
            continue;
        }

        if (headerSection)
        {
            std::string::size_type separator = line.find(':');
            if (separator != std::string::npos)
            {
                std::string key = line.substr(0, separator);
                std::string value = line.substr(separator + 2);
                response.headers[key] = value;
            }
        }
        else
        {
            response.body += line + "\n";
        }
    }

    if (response.headers.find("Status") == response.headers.end())
    {
        response.statusCode = 200;
        response.headers["Status"] = "200 OK";
    }

    if (response.headers.find("Content-Type") == response.headers.end())
    {
        response.headers["Content-Type"] = "text/html; charset=utf-8";
    }

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    return response;
}
