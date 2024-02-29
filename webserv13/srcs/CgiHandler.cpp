#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const std::string& scriptPath, const HttpRequest& request)
: scriptPath(scriptPath), request(request) {
    std::cerr << "Constructeur -> CgiHandler constructed with scriptPath: " << scriptPath << std::endl;
}

CgiHandler::~CgiHandler() {
    std::cerr << "Destructeur -> CgiHandler destructed" << std::endl;
}


HttpResponse CgiHandler::executeScript() {
    setupEnvironment();

    int pipefd[2];
    pid_t pid;
    if (pipe(pipefd) == -1) {
        std::cerr << "executeScript -> Erreur lors de la création du pipe: " << strerror(errno) << std::endl;
        return HttpResponse();
    }

    pid = fork();
    if (pid == -1) {
        std::cerr << "executeScript -> Erreur lors de l'exécution de fork(): " << strerror(errno) << std::endl;
        return HttpResponse();
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        std::vector<std::string> envStrings;
        for (std::map<std::string, std::string>::iterator it = cgiEnvironment.begin(); it != cgiEnvironment.end(); ++it) {
            envStrings.push_back(it->first + "=" + it->second);
        }
        std::vector<char*> envp(envStrings.size() + 1, NULL);
        for (size_t i = 0; i < envStrings.size(); ++i) {
            envp[i] = const_cast<char*>(envStrings[i].c_str());
        }
        envp[envStrings.size()] = NULL;

        char* argv[] = {const_cast<char*>(scriptPath.c_str()), NULL};

        std::cerr << "executeScript -> Executing script: " << scriptPath << std::endl;
        if (execve(scriptPath.c_str(), argv, &envp[0]) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        close(pipefd[1]);

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output.append(buffer);
        }

        waitpid(pid, NULL, 0);

        return parseCgiOutput(output);
    }

    return HttpResponse();
}


void CgiHandler::setupEnvironment()
{
    //nettoyage PATH_INFO
    //nettoyage QUERY_STRING

    cgiEnvironment["REQUEST_METHOD"] = request.method;
    cgiEnvironment["QUERY_STRING"] = request.queryString;
    cgiEnvironment["CONTENT_TYPE"] = request.getHeader("Content-Type");
    cgiEnvironment["CONTENT_LENGTH"] = request.getHeader("Content-Length");
    cgiEnvironment["SCRIPT_FILENAME"] = scriptPath;
    //cgiEnvironment["SCRIPT_NAME"] = ...;
    //cgiEnvironment["PATH_INFO"] = ...;
    //DOCUMENT_ROOT
    //SERVER_NAME
    //SERVER_PORT ??

}

HttpResponse CgiHandler::parseCgiOutput(const std::string& output)
{
    HttpResponse response;
    std::string line;
    bool headerSection = true;
    std::istringstream stream(output.c_str());
    std::cerr << "parseCgiOutput -> Début de l'analyse de la sortie CGI." << std::endl;

    while (stream.good())
    {
        std::getline(stream, line, '\n');
        std::cerr << "parseCgiOutput -> Ligne lue: " << line << std::endl;
        if (line.empty() || line == "\r")
        {
            if (line == "\r")
            {
                headerSection = false;
                std::cerr << "parseCgiOutput -> Fin des en-têtes, début du corps." << std::endl;
            }
            continue;
        }

        if (headerSection) {
            std::string::size_type colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 2);
                response.headers[headerName] = headerValue;
                std::cerr << "parseCgiOutput -> En-tête ajouté: " << headerName << ": " << headerValue << std::endl;
            }
        } else {
            if (!response.body.empty()) response.body += "\n";
            response.body += line;
            std::cerr << "parseCgiOutput -> Ajout au corps: " << line << std::endl;
        }
    }

    std::cerr << "parseCgiOutput -> Analyse terminée. Statut: " << response.statusCode << std::endl;
    return response;
}



