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

void Server::start()
{
    std::cout << "Server starting on port: " << config.get("port") << std::endl;

    addrlen = sizeof(address);
    while (true)
    {
        std::cout << "Waiting for new connections..." << std::endl;
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0)
        {
            std::cerr << "Accept failed" << std::endl;
            continue; // Continue d'écouter les nouvelles connexions même en cas d'erreur
        }

        // Traitement de la connexion entrante
        char buffer[1024] = {0};
        ssize_t bytes_read = read(new_socket, buffer, sizeof(buffer) - 1); // Laisse un octet pour le caractère nul de fin
        if (bytes_read < 0)
        {
            std::cerr << "Error reading from socket" << std::endl;
            close(new_socket);
            continue;
        }

        // Parse la requête HTTP reçue
        HttpRequest httpRequest = requestHandler.parseRequest(std::string(buffer, bytes_read));
        
        // Traite la requête et génère une réponse
        HttpResponse httpResponse = requestHandler.handleRequest(httpRequest);
        
        // Construit la réponse HTTP en une chaîne de caractères
        std::string responseText = response.buildHttpResponse(httpResponse);
        
        // Envoie la réponse au client
        write(new_socket, responseText.c_str(), responseText.size());
        
        std::cout << "Response sent." << std::endl;
        close(new_socket);
    }
}

