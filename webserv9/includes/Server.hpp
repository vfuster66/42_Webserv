#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "ConfigParser.hpp"
#include "StaticRequestHandler.hpp"
#include "Response.hpp"
#include "CgiRequestHandler.hpp"
#include "HttpUtils.hpp"

struct HttpRequest;
struct HttpResponse;

class Server
{
private:
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int addrlen;
    std::string basePath;
    ConfigParser config;
    RequestHandler* requestHandler;
    Response response;

    Server(const Server &other);
    Server &operator=(const Server &other);

    void setupRequestHandler()
    {
        std::string handlerType = config.get("handlerType");

        if (handlerType == "static")
        {
            requestHandler = new StaticRequestHandler();
        }
        else if (handlerType == "cgi")
        {
            requestHandler = new CgiRequestHandler();
        }
        else
        {
            std::cerr << "Type de gestionnaire non supporté spécifié dans le fichier de configuration: " << handlerType << std::endl;
            exit(1);
        }
    }


    void setupServerSocket();

public:
    Server(const std::string& configFilePath);


    ~Server();

    void start();

};

#endif
