#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Logger.hpp"

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <climits>

struct LocationConfig {
    std::string                             path;
    bool                                    is_regex;
    std::string                             root;
    bool                                    autoindex;
    std::vector<std::string>                allowed_methods;
    std::vector<std::string>                allowed_ips;
    std::vector<std::string>                denied_ips;
    std::vector<std::string>                index;
    std::string                             cgi_path;

    LocationConfig() : autoindex(false) {}
};

struct ServerConfig {
    std::string                             host;
    int                                     port;
    std::vector<std::string>                server_names;
    std::string                             error_page;
    int                                     client_max_body_size;
    std::string                             root;
    std::vector<std::string>                index;
    std::vector<std::string>                allowed_ips;
    std::vector<std::string>                denied_ips;
    std::vector<LocationConfig>             locations;
};


class ConfigParser
{
private:
    std::string                 filename;
    // Utilisation d'un vecteur pour stocker les configurations de plusieurs serveurs
    std::vector<ServerConfig>   servers;    
    int serverCount;
    std::istringstream iss;

    // Empêche la copie
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

    // Méthodes utilitaires pour le parsing
    void resetISS(std::istringstream& iss, const std::string& newStr);
    void trim(std::string& str) const;
    std::string removeQuotes(const std::string& input) const;
    int  convertSizeToBytes(const std::string& sizeStr) const;

    // Méthodes pour parser un bloc de serveur
    void parseServerBlock(std::ifstream& configFile, ServerConfig& serverConfig);
    void parseKeyValue(const std::string& line, ServerConfig& serverConfig);

    // Méthodes pour parser un bloc location a l'interieur d'un bloc serveur
    void parseLocationBlock(std::ifstream& configFile, ServerConfig& serverConfig);
    void parseLocationLine(std::istringstream& lineStream, LocationConfig& locationConfig);

public:
    explicit ConfigParser(const std::string& filename, const std::string& logFile, Logger::Level logLevel);
    ~ConfigParser();

    void        parse();

    // Méthodes pour accéder aux configurations des serveurs
    const std::vector<ServerConfig>& getServers() const;
    std::string get(const std::string& key) const;

};

#endif




