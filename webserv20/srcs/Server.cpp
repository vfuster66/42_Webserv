#include "../includes/Server.hpp"

bool Server::isRunning = true;

Server::Server(const std::string& configFilePath, const std::string& logFilePath, Logger::Level logLevel)
: config(configFilePath, logFilePath, logLevel)
{
    LOG_INFO("Initialisation du serveur avec le fichier de configuration : " + configFilePath);
    
    config.parse();
    
    requestHandler.setServerConfigs(config.getServers());
    
    setupServerSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < server_fds.size(); ++i)
    {
        if (server_fds[i] != -1)
        {
            close(server_fds[i]);
        }
    }
    server_fds.clear();
    
    LOG_INFO("Sockets serveur fermés avec succès.");
}

void Server::shutdownServer(const std::string& reason)
{
    LOG_ERROR("Arrêt du serveur en raison de : " + reason);
    for (size_t i = 0; i < server_fds.size(); ++i) 
    {
        int fd = server_fds[i];
        if (fd != -1) close(fd);
    }

    server_fds.clear();

    exit(EXIT_FAILURE);
}

void Server::setupServerSockets()
{
    server_fds.clear();

    const std::vector<ServerConfig>& servers = config.getServers();

    for (size_t i = 0; i < servers.size(); ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1)
        {
            std::ostringstream oss;
            oss << "Échec de la création du socket pour le serveur à l'index " << i;
            LOG_ERROR(oss.str());
            continue; 
        }

        int opt = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::ostringstream oss;
            oss << "Échec de la configuration de SO_REUSEADDR pour le port: " << servers[i].port;
            LOG_ERROR(oss.str());
            close(fd);
            continue;
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(static_cast<uint16_t>(servers[i].port));

        if (bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            std::ostringstream oss;
            oss << "Échec du bind sur le port: " << servers[i].port;
            LOG_ERROR(oss.str());
            close(fd);
            continue;
        }

        if (listen(fd, 100) < 0)
        {
            std::ostringstream oss;
            oss << "Échec de l'écoute sur le port: " << servers[i].port;
            LOG_ERROR(oss.str());
            shutdownServer("Impossible de démarrer l'écoute sur tous les ports requis.");
        }

        server_fds.push_back(fd);

        std::ostringstream oss;
        oss << "Socket serveur configuré et en écoute sur le port " << servers[i].port;
        LOG_INFO(oss.str());
    }
}

void Server::safeWrite(int fd, const std::string& data)
{
    ssize_t totalWritten = 0;
    ssize_t dataLength = data.size();
    const char* dataPtr = data.c_str();

    while (totalWritten < dataLength)
    {
        ssize_t bytesWritten = write(fd, dataPtr + totalWritten, dataLength - totalWritten);
        if (bytesWritten < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            LOG_ERROR("Échec de l'envoi des données au client.");
            close(fd);
            return;
        }
        totalWritten += bytesWritten;
    }
}

void Server::start()
{
    fd_set master_set, read_fds;
    int max_fd = 0;

    FD_ZERO(&master_set);
    for (std::vector<int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it)
    {
        int fd = *it;
        FD_SET(fd, &master_set);
        if (fd > max_fd)
        {
            max_fd = fd;
        }
    }

    LOG_INFO("Serveur démarré et en attente de connexions sur plusieurs ports...");

    std::time_t lastCleanupTime = std::time(0);
    int cleanupInterval = 60;

    while (isRunning)
    {
        read_fds = master_set;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            LOG_ERROR("Erreur lors de l'exécution de select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (std::find(server_fds.begin(), server_fds.end(), i) != server_fds.end())
                {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(i, (sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0)
                    {
                        LOG_ERROR("Erreur lors de l'acceptation d'une nouvelle connexion");
                    }
                    else
                    {
                        FD_SET(client_fd, &master_set);
                        if (client_fd > max_fd)
                        {
                            max_fd = client_fd;
                        }
                        std::ostringstream oss;
                        oss << "Nouvelle connexion depuis " << inet_ntoa(client_addr.sin_addr);
                        LOG_INFO(oss.str());
                    }
                }
                else
                {
                    char buffer[1024] = {0};
                    ssize_t bytes_read = read(i, buffer, sizeof(buffer) - 1);
                    if (bytes_read <= 0)
                    {
                        close(i);
                        FD_CLR(i, &master_set);
                    }
                    else
                    {
                        // Création de l'objet Cookies et extraction des cookies de la requête
                        Cookies cookies;
                        std::string requestStr(buffer, bytes_read);

                        // Log avant extraction
                        LOG_INFO("Extracting cookies from request: " + requestStr);

                        cookies.extractCookiesFromRequest(requestStr);

                        // Tentative de récupération du sessionId
                        std::string sessionId = cookies.getValue("sessionId");

                        // Log après tentative de récupération
                        LOG_INFO("Retrieved sessionId from cookies: " + sessionId);

                        // Validation et gestion de la session
                        if (!sessionId.empty() && sessionManager.validateSession(sessionId))
                        {
                            LOG_INFO("Session valid. Updating last activity for sessionId: " + sessionId);
                            sessionManager.updateLastActivity(sessionId);
                        }
                        else
                        {
                            LOG_INFO("Session invalid or not found. Creating new session.");
                            sessionId = sessionManager.createSession();
                            int cookieMaxAge = 3600;
                            cookies.setValue("sessionId", sessionId, false, "/", cookieMaxAge);

                            // Log après la création d'une nouvelle session
                            std::ostringstream oss;
                            oss << "New session created with sessionId: " << sessionId << ". Max Age: " << cookieMaxAge;
                            LOG_INFO(oss.str());

                        }


                        HttpRequest httpRequest = requestHandler.parseRequest(requestStr);
                        HttpResponse httpResponse = requestHandler.handleRequest(httpRequest);

                        httpResponse.headers["Set-Cookie"] = cookies.toString();

                        std::string responseText = Response::buildHttpResponse(httpResponse);
                        safeWrite(i, responseText);

                        std::string connectionHeader = httpRequest.getHeader("Connection");
                        if (connectionHeader != "keep-alive")
                        {
                            close(i);
                            FD_CLR(i, &master_set);
                        }
                    }
                }
            }
        }
    }
    std::time_t now = std::time(0);
    if (now - lastCleanupTime > cleanupInterval)
    {
        sessionManager.cleanupExpiredSessions(3600);
        lastCleanupTime = now;
    }
}

