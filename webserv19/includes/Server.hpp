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
#include "Structures.hpp"
#include "SessionManager.hpp"
#include "Cookies.hpp"

class Server
{
private:

    static bool         isRunning;
    std::vector<int>    server_fds;
    int                 new_socket;
    int                 addrlen;
    struct sockaddr_in  address;
    std::string         basePath;

    ConfigParser        config;
    RequestHandler      requestHandler;
    Response            response;

    Server(const Server &other);
    Server &operator=(const Server &other);

    void shutdownServer(const std::string& reason);
    void setupServerSockets();
    void safeWrite(int fd, const std::string& data);

public:

    Server(const std::string& configFilePath, const std::string& logFilePath, Logger::Level logLevel = Logger::INFO);
    ~Server();

    void start();

};

#endif
