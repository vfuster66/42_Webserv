#include "Server.hpp"

Server::Server(const std::string& configFilePath) : config(configFilePath)
{
    // Parse le fichier de configuration dès la création de l'objet Server
    config.parse();

    // Configuration du serveur basée sur les valeurs du fichier de configuration
    setupServerSocket();
}

Server::~Server()
{
    if (server_fd != -1)
    {
        close(server_fd);
    }
}

void Server::setupServerSocket()
{
    // Création du socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    address.sin_family = AF_INET;
    // Écoute sur toutes les interfaces
    address.sin_addr.s_addr = INADDR_ANY;

    // Lecture du port depuis le fichier de configuration
    int port = std::atoi(config.get("port").c_str());
    address.sin_port = htons(port);

    // Attache le socket à l'adresse et au port spécifiés
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Démarre l'écoute sur le socket
    // La file d'attente peut gérer jusqu'à 10 connexions
    if (listen(server_fd, 10) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::start() {
    fd_set master_set, read_fds;
    int max_fd;

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_fd = server_fd;

    std::cout << "Server starting on port: " << config.get("port") << std::endl;

    while (true) {
        read_fds = master_set; // Prépare read_fds pour select
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for (int i = 0; i <= max_fd; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_fd) {
                    // Nouvelle connexion
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        perror("accept");
                    } else {
                        FD_SET(client_fd, &master_set); // Ajoute le nouveau socket client à l'ensemble
                        if (client_fd > max_fd) {
                            max_fd = client_fd;
                        }
                        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << std::endl;
                    }
                } else {
                    // Données à lire sur un socket client
                    char buffer[1024] = {0};
                    ssize_t bytes_read = read(i, buffer, sizeof(buffer) - 1);
                    if (bytes_read <= 0) {
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        HttpRequest httpRequest = requestHandler.parseRequest(std::string(buffer, bytes_read));
                        HttpResponse httpResponse = requestHandler.handleRequest(httpRequest);

                        std::string connectionHeader = RequestHandler::getHeader(httpRequest, "Connection");
                        if (connectionHeader == "keep-alive") {
                            httpResponse.headers["Connection"] = "keep-alive";
                        } else {
                            httpResponse.headers["Connection"] = "close";
                        }

                        std::string responseText = Response::buildHttpResponse(httpResponse);
                        write(i, responseText.c_str(), responseText.size());

                        if (connectionHeader != "keep-alive") {
                            close(i);
                            FD_CLR(i, &master_set);
                        }
                    }
                }
            }
        }
    }
}


