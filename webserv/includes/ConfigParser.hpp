#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept> // Pour std::runtime_error

class ConfigParser {
private:
    std::string filename;
    std::map<std::string, std::string> configMap;

public:
    explicit ConfigParser(const std::string& filename);
    ConfigParser(const ConfigParser& other); // Constructeur de copie
    ConfigParser& operator=(const ConfigParser& other); // Opérateur d'affectation
    ~ConfigParser(); // Destructeur

    void parse();
    std::string get(const std::string& key) const;

private:
    std::string trim(const std::string& str) const; // Fonction pour trimmer les espaces
};

#endif // CONFIGPARSER_HPP


