#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "ConfigParser.hpp"
#include "RequestHandler.hpp"
#include "Response.hpp"
#include "Logger.hpp"

class Server
{
private:
    std::vector<int> server_fds;
    int new_socket;
    struct sockaddr_in address;
    int addrlen;
    std::string basePath;
    ConfigParser config;
    RequestHandler requestHandler;
    Response response;

    Server(const Server &other);
    Server &operator=(const Server &other);

    void setupServerSockets();

public:
    Server(const std::string& configFilePath, const std::string& logFilePath, Logger::Level logLevel = Logger::INFO);

    ~Server();

    void start();

};

#endif

