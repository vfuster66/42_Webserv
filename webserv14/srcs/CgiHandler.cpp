#include "CgiHandler.hpp"

CgiHandler::CgiHandler(const std::string& scriptPath, const HttpRequest& request)
: scriptPath(scriptPath), request(request) {
    LOG_INFO("Constructeur -> Initialisation de CgiHandler avec le script: " + scriptPath);
    setupEnvironment();
}

CgiHandler::~CgiHandler() {
    LOG_INFO("Destructeur -> Destruction de CgiHandler");
}

void CgiHandler::setupEnvironment() {
    LOG_INFO("setupEnvironment -> Configuration de l'environnement CGI pour le script: " + scriptPath);

    cgiEnvironment["REQUEST_METHOD"] = request.method;
    LOG_INFO("setupEnvironment -> Méthode de requête: " + request.method);

    cgiEnvironment["QUERY_STRING"] = request.queryString;
    LOG_INFO("setupEnvironment -> Chaîne de requête: " + request.queryString);

    cgiEnvironment["CONTENT_TYPE"] = request.getHeader("Content-Type");
    LOG_INFO("setupEnvironment -> Type de contenu: " + request.getHeader("Content-Type"));

    cgiEnvironment["CONTENT_LENGTH"] = request.getHeader("Content-Length");
    LOG_INFO("setupEnvironment -> Longueur du contenu: " + request.getHeader("Content-Length"));

    cgiEnvironment["SCRIPT_FILENAME"] = scriptPath;
    LOG_INFO("setupEnvironment -> Chemin du script CGI: " + scriptPath);

}

HttpResponse CgiHandler::executeScript() {
    LOG_INFO("executeScript -> Début de l'exécution du script CGI.");

    std::ifstream scriptFile(scriptPath.c_str());
    std::string scriptContents((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
    scriptFile.close();
    LOG_INFO(std::string("Contenu du script PHP à exécuter : \n") + scriptContents);


    int pipefd[2];
    if (pipe(pipefd) != 0) {
        LOG_ERROR("executeScript -> Erreur lors de la création du pipe.");
        return HttpResponse();
    }
    LOG_INFO("executeScript -> Pipe créé avec succès.");

    pid_t pid = fork();
    if (pid == -1) {
        LOG_ERROR("executeScript -> Erreur lors de l'exécution de fork().");
        close(pipefd[0]);
        close(pipefd[1]);
        return HttpResponse();
    }

    if (pid == 0) {
        LOG_INFO("executeScript -> Dans le processus enfant.");
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        LOG_INFO("executeScript -> Sorties stdout et stderr redirigées.");

        const char* argv[] = {scriptPath.c_str(), NULL};
        char** envp = new char*[cgiEnvironment.size() + 1];
        std::map<std::string, std::string>::iterator it;
        int i = 0;
        for (it = cgiEnvironment.begin(); it != cgiEnvironment.end(); ++it, ++i) {
            std::string env = it->first + "=" + it->second;
            envp[i] = strdup(env.c_str());
            LOG_INFO(std::string("executeScript -> Variable d'environnement ajoutée : ") + env);
        }
        envp[i] = NULL;

        execve(scriptPath.c_str(), const_cast<char* const*>(argv), envp);
        LOG_ERROR("executeScript -> Échec de execve. Sortie du processus enfant avec erreur.");
        for (size_t i = 0; i < cgiEnvironment.size(); ++i) {
            free(envp[i]);
        }
        delete[] envp;
        exit(EXIT_FAILURE);
    } else {
        LOG_INFO("executeScript -> Dans le processus parent. Attente de la sortie du script.");
        close(pipefd[1]);
        std::string output;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }
        close(pipefd[0]);
        LOG_INFO("executeScript -> Lecture de la sortie du script terminée. Sortie du script :\n" + output);
        waitpid(pid, NULL, 0);
        LOG_INFO("executeScript -> Processus enfant terminé. Traitement de la sortie du script.");

        return parseCgiOutput(output);
    }
}

HttpResponse CgiHandler::parseCgiOutput(const std::string& cgiOutput) {
    HttpResponse response;

    // Logger un aperçu de la sortie CGI au début de la fonction
    const size_t previewLength = 500; // Nombre maximal de caractères à logger
    std::string preview = cgiOutput.substr(0, previewLength);
    if (cgiOutput.size() > previewLength) {
        preview += "... (suite omise pour la clarté des logs)";
    }
    LOG_INFO(std::string("parseCgiOutput -> Aperçu de la sortie CGI : \n") + preview);

    std::istringstream stream(cgiOutput);
    std::string line;
    bool headerSection = true;

    while (getline(stream, line)) {
        
        if (!headerSection) {
            response.body += line + "\n";
            LOG_INFO(std::string("parseCgiOutput -> Ligne ajoutée au body CGI : ") + line);
            continue;
        }

        if (line.empty()) {
            if (headerSection) {
                headerSection = false;
                LOG_INFO("parseCgiOutput -> Fin des headers CGI. Début du body.");
                continue;
            }
        } else if (headerSection) {
            LOG_INFO(std::string("parseCgiOutput -> Header CGI récupéré : ") + line);
            std::string::size_type separator = line.find(':');
            if (separator != std::string::npos) {
                std::string key = line.substr(0, separator);
                std::string value = line.substr(separator + 2);
                if (key == "Status")
                {
                    response.statusCode = atoi(value.c_str());
                    LOG_INFO(std::string("parseCgiOutput -> StatusCode spécifié : ") + value);
                }
                response.headers[key] = value;
            }
        }
    }


    if (response.statusCode == 0) {
        LOG_INFO("parseCgiOutput -> Aucun statusCode spécifié. Utilisation de la valeur par défaut : 200");
        response.statusCode = 200;
    }
    if (response.headers.find("Content-Type") == response.headers.end()) {
        LOG_INFO("parseCgiOutput -> Aucun Content-Type spécifié. Utilisation de la valeur par défaut : text/html");
        response.headers["Content-Type"] = "text/html";
    }

    std::ostringstream oss;
    oss << response.body.size();
    response.headers["Content-Length"] = oss.str();

    LOG_INFO(std::string("parseCgiOutput -> Taille du body CGI : ") + oss.str());

    return response;
}



