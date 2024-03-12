#include <csignal>

#include "../includes/Server.hpp"

std::vector<int> openSockets;
std::vector<ServerConfig> serverConfigs;

bool isRunning = true;

void cleanup(int signum)
{

    for (size_t i = 0; i < openSockets.size(); ++i)
    {
        if (close(openSockets[i]) == -1)
        {
            std::cerr << "Erreur lors de la fermeture du descripteur de fichier " << openSockets[i] << std::endl;
        }
    }
    openSockets.clear();
    serverConfigs.clear();

    Logger::getInstance().cleanup();

    exit(signum);
}

void handleSignal(int signum)
{
    switch (signum)
    {
        case SIGINT:
        case SIGTERM:
            std::cout << "\r" << "Signal de terminaison reçu. Arrêt du serveur..." << std::endl;
            isRunning = false;
            cleanup(signum);
            break;
    }
}
void setupSignalHandlers()
{
    struct sigaction sa;

    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char *argv[])
{
    setupSignalHandlers();

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    std::string configFilePath = argv[1];
    std::string logFilePath = "server.log";
    Logger::Level logLevel = Logger::INFO;

    Server httpServer(configFilePath, logFilePath, logLevel);
    httpServer.start();

    return 0;
}

