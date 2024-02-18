#include "CgiRequestHandler.hpp"


HttpResponse CgiRequestHandler::handleRequest(const HttpRequest& request)
{
    HttpResponse response;

    int cgiOutput[2];
    if (pipe(cgiOutput) == -1)
    {
        response.statusCode = 500; // Internal Server Error
        response.body = "Erreur interne du serveur: Impossible de créer un pipe.";
        return response;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        response.statusCode = 500; // Internal Server Error
        response.body = "Erreur interne du serveur: Impossible de créer un processus enfant.";
        close(cgiOutput[0]); close(cgiOutput[1]);
        return response;
    }

    if (pid == 0)
    { // Processus enfant
        if (dup2(cgiOutput[1], STDOUT_FILENO) == -1)
	{
            // En cas d'échec de dup2, sortir avec un code d'erreur spécifique
            exit(1);
        }
        close(cgiOutput[0]); close(cgiOutput[1]);

        char* argv[] = { /* Chemin du script CGI, arguments, NULL */ };
        char* envp[] = { /* Variables d'environnement pour le CGI, NULL */ };

        execve(argv[0], argv, envp);
        // Si execve échoue, sortir avec un code d'erreur différent pour indiquer ce type d'échec
        exit(2);
    }
    else
    { // Processus parent
        close(cgiOutput[1]); // Fermer l'extrémité d'écriture inutilisée dans le parent

        int status;
        waitpid(pid, &status, 0); // Attendre la fin du processus enfant

        // Vérifier si le processus enfant a terminé normalement
        if (WIFEXITED(status))
	{
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 1)
	    {
                response.statusCode = 500; // Internal Server Error
                response.body = "Erreur interne du serveur: Échec de dup2.";
                close(cgiOutput[0]);
                return response;
            }
	    else if (exitStatus == 2)
	    {
                response.statusCode = 500; // Internal Server Error
                response.body = "Erreur interne du serveur: Échec de execve.";
                close(cgiOutput[0]);
                return response;
            }
        }

        std::stringstream cgiResponse;
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(cgiOutput[0], buffer, sizeof(buffer))) > 0)
	{
            cgiResponse.write(buffer, bytesRead);
        }
        close(cgiOutput[0]);

        if (bytesRead == -1)
	{
            response.statusCode = 500; // Internal Server Error
            response.body = "Erreur interne du serveur: Échec de la lecture du pipe.";
            return response;
        }

        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        response.body = cgiResponse.str();
        response.headers["Content-Type"] = "text/html"; // Supposition pour cet exemple
    }

    return response;
}

