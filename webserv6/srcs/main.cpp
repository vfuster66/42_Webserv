#include "../includes/Server.hpp"

int main(int ac, char *av[])
{
    try
    {
        if (ac != 2)
        {
            std::cerr << "Usage: " << av[0] << " <config_file_path>" << std::endl;
            return 1;
        }

        std::string configFilePath = av[1];
        std::string logFilePath = "server.log";
        Logger::Level logLevel = Logger::INFO;

        Server httpServer(configFilePath, logFilePath, logLevel);

        httpServer.start();

    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur lors du démarrage du serveur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}




