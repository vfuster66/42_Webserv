#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "ConfigParser.hpp"
#include "RequestHandler.hpp"
#include "Response.hpp"

class Server
{
private:
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int addrlen;
    std::string basePath;
    ConfigParser config;
    RequestHandler requestHandler;
    Response response;

    Server(const Server &other);
    Server &operator=(const Server &other);

    void setupServerSocket();

public:
    Server(const std::string& configFilePath);

    ~Server();

    void start();

};

#endif
