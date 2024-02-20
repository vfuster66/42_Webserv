#include "../includes/Server.hpp"

Server::Server(const std::string& configFilePath, const std::string& logFilePath, Logger::Level logLevel)
    : config(configFilePath, logFilePath, logLevel)
{
    LOG_INFO("Server constructor -> Initialisation du serveur avec le fichier de configuration : " + configFilePath);
    config.parse();
    setupServerSockets();
}

Server::~Server()
{
    for (std::vector<int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it)
    {
        if (*it != -1)
        {
            close(*it);
        }
    }
    LOG_INFO("Server destructor -> Sockets serveur fermés avec succès.");
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
            oss << "setupServerSockets -> Échec de la création du socket pour le serveur à l'index " << i;
            LOG_ERROR(oss.str());
            continue; 
        }

        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(static_cast<uint16_t>(servers[i].port));

        if (bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            close(fd);
            std::ostringstream oss;
            oss << "setupServerSockets -> Échec du bind sur le port: " << servers[i].port;
            LOG_ERROR(oss.str());
            continue;
        }

        if (listen(fd, 100) < 0)
        {
            close(fd);
            std::ostringstream oss;
            oss << "setupServerSockets -> Échec de l'écoute sur le port: " << servers[i].port;
            LOG_ERROR(oss.str());
            continue;
        }

        server_fds.push_back(fd);

        std::ostringstream oss;
        oss << "setupServerSockets -> Socket serveur configuré et en écoute sur le port " << servers[i].port;
        LOG_INFO(oss.str());
    }
}

void Server::start() {
    fd_set master_set, read_fds;
    int max_fd = 0;

    FD_ZERO(&master_set);

    // Ajoutez tous les sockets d'écoute au master_set et mettez à jour max_fd
    for (std::vector<int>::iterator it = server_fds.begin(); it != server_fds.end(); ++it) {
        FD_SET(*it, &master_set);
        if (*it > max_fd) {
            max_fd = *it;
        }
    }

    LOG_INFO("start -> Serveur démarré et en attente de connexions sur plusieurs ports...");

    while (true) {
        read_fds = master_set;
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            LOG_ERROR("start -> Erreur lors de l'exécution de select");
            exit(EXIT_FAILURE);
        }

        // Vérifiez chaque socket pour voir s'il est prêt
        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // Nouvelle connexion sur un des sockets d'écoute
                if (std::find(server_fds.begin(), server_fds.end(), i) != server_fds.end()) {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(i, (sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        LOG_ERROR("start -> Erreur lors de l'acceptation d'une nouvelle connexion");
                    } else {
                        FD_SET(client_fd, &master_set);
                        if (client_fd > max_fd) {
                            max_fd = client_fd;
                        }
                        std::ostringstream oss;
                        oss << "start -> Nouvelle connexion depuis " << inet_ntoa(client_addr.sin_addr);
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
                        HttpRequest httpRequest = requestHandler.parseRequest(std::string(buffer, bytes_read));
                        HttpResponse httpResponse = requestHandler.handleRequest(httpRequest);

                        std::string connectionHeader = RequestHandler::getHeader(httpRequest, "Connection");
                        if (connectionHeader == "keep-alive")
                        {
                            httpResponse.headers["Connection"] = "keep-alive";
                        } else {
                            httpResponse.headers["Connection"] = "close";
                        }

                        std::string responseText = Response::buildHttpResponse(httpResponse);
                        write(i, responseText.c_str(), responseText.size());

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
}


