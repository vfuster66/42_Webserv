#include "Server.hpp"
#include "ConfigParser.hpp"

int main(int ac, char *av[])
{
    try
    {
        // Vérification des arguments du programme, si nécessaire
        if (ac != 2)
        {
            std::cerr << "Usage: " << av[0] << " <config_file_path>" << std::endl;
            return 1;
        }

        // Initialisation du serveur avec le fichier de configuration
        std::string configFilePath = av[1];
        Server httpServer(configFilePath);

        // Démarrage du serveur
        httpServer.start();

        std::cout << "Serveur HTTP démarré avec succès." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Erreur lors du démarrage du serveur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
