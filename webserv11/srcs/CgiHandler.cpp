#include "../includes/CgiHandler.hpp"

// Constructeur
CgiHandler::CgiHandler(const std::string& scriptPath, const std::map<std::string, std::string>& cgiEnv)
    : scriptPath(scriptPath), cgiEnv(cgiEnv)
{
    LOG_INFO("CgiHandler constructeur -> CgiHandler créé pour le script " + scriptPath);
}

// Constructeur par copie
CgiHandler::CgiHandler(const CgiHandler& other)
    : scriptPath(other.scriptPath), cgiEnv(other.cgiEnv)
{
    // Logique de copie supplémentaire si nécessaire
}

// Destructeur
CgiHandler::~CgiHandler()
{
    // Nettoyage des ressources allouées dynamiquement si nécessaire
}

// Opérateur d'assignation
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

std::string CgiHandler::getScriptPath() const
{
    return scriptPath;
}

void CgiHandler::setEnvironmentVariables()
{
    for (std::map<std::string, std::string>::const_iterator it = cgiEnv.begin(); it != cgiEnv.end(); ++it)
    {
        setenv(it->first.c_str(), it->second.c_str(), 1);
        LOG_INFO("setEnvironmentVariables -> Variable d'environnement CGI définie: " + it->first + "=" + it->second);
    }

    setenv("REDIRECT_STATUS", "200", 1);
}



std::string CgiHandler::execute()
{
    LOG_INFO("execute -> Début de l'exécution du script CGI: " + scriptPath);

    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        LOG_ERROR("execute -> Erreur lors de la création du pipe pour le CGI");
        return "";
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        LOG_ERROR("execute -> Échec du fork pour le CGI");
        return "";
    }
    else if (pid == 0)
    {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            LOG_ERROR("execute -> Erreur lors de la redirection de STDOUT pour le CGI");
            exit(EXIT_FAILURE);
        }
        setEnvironmentVariables();
        execl("/usr/bin/php-cgi", "php-cgi", scriptPath.c_str(), (char *)NULL);
        LOG_ERROR("execute -> Échec de l'exécution du script CGI");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipefd[1]);
        waitpid(pid, NULL, 0);

        std::string output;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }
        close(pipefd[0]);

        LOG_INFO("execute -> Script CGI exécuté avec succès: " + scriptPath);
        return output;
    }
}
