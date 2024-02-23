#include "../includes/Server.hpp"
#include "../includes/ConfigParser.hpp"
#include "../includes/CgiRequestHandler.hpp"
#include "../includes/HttpUtils.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpResponse.hpp"


#include <csignal>

std::vector<int> openSockets;
std::vector<LocationConfig> locationConfigs;
std::vector<ServerConfig> serverConfigs;

bool isRunning = true;

void cleanup(int signum) {

    for (size_t i = 0; i < openSockets.size(); ++i)
    {
        if (close(openSockets[i]) == -1)
        {
            std::cerr << "Erreur lors de la fermeture du descripteur de fichier " << openSockets[i] << std::endl;
        }
    }
    openSockets.clear();
    locationConfigs.clear();
    serverConfigs.clear();

    Logger::getInstance().cleanup();

    exit(signum);
}

void handleSignal(int signum) {
    (void)signum;
    std::cout << "Signal de terminaison reçu. Arrêt du serveur..." << std::endl;
    cleanup(signum);
    isRunning = false;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handleSignal);

    if (argc != 2) {
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
