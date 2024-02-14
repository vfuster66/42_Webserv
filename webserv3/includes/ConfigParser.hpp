#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

/**
 * Classe ConfigParser
 * Permet de parser un fichier de configuration et de récupérer 
 * des valeurs de configuration spécifiques.
 */

class ConfigParser
{
private:

    // Le chemin vers le fichier de configuration.
    std::string filename;
    // Stocke les paires clé-valeur de configuration.
    std::map<std::string, std::string> configMap;

    /**
     * Supprime les espaces au début et à la fin d'une chaîne de caractères.
     * @param str La chaîne de caractères à trimmer.
     * @return La chaîne trimmée.
     */

    std::string trim(const std::string& str) const;

    /**
     * Supprime les guillemets au début et à la fin d'une chaîne de caractères.
     * @param input La chaîne de caractères à nettoyer.
     * @return La chaîne nettoyée.
     */

    std::string removeQuotes(const std::string& input) const;

public:

    /**
     * Constructeur explicite.
     * @param filename Le chemin vers le fichier de configuration à parser.
     */

    explicit ConfigParser(const std::string& filename);

    /**
     * Constructeur de copie.
     * @param other Une autre instance de ConfigParser à copier.
     */

    ConfigParser(const ConfigParser& other);

    /**
     * Opérateur d'affectation.
     * @param other Une autre instance de ConfigParser à affecter.
     * @return Une référence à cette instance de ConfigParser.
     */

    ConfigParser& operator=(const ConfigParser& other); // Opérateur d'affectation

    /**
     * Destructeur.
     */

    ~ConfigParser(); // Destructeur

    /**
     * Parse le fichier de configuration et remplit configMap avec les paires clé-valeur trouvées.
     * @throws std::runtime_error Si le fichier ne peut pas être ouvert ou s'il y a un problème de format.
     */

    void parse();

    /**
     * Récupère une valeur de configuration comme booléen.
     * @param key La clé de la valeur booléenne à récupérer.
     * @return La valeur booléenne associée à la clé.
     * @throws std::runtime_error Si la clé n'existe pas ou si la valeur ne peut pas être convertie en booléen.
     */

    bool getBool(const std::string& key) const;

    /**
     * Récupère une valeur de configuration comme entier.
     * @param key La clé de la valeur entière à récupérer.
     * @return La valeur entière associée à la clé.
     * @throws std::runtime_error Si la clé n'existe pas ou si la valeur ne peut pas être convertie en entier.
     */

    int getInt(const std::string& key) const;

    /**
     * Récupère une valeur de configuration comme flottant.
     * @param key La clé de la valeur flottante à récupérer.
     * @return La valeur flottante associée à la clé.
     * @throws std::runtime_error Si la clé n'existe pas ou si la valeur ne peut pas être convertie en flottant.
     */

    float getFloat(const std::string& key) const;

    /**
     * Récupère une valeur de configuration comme chaîne de caractères.
     * @param key La clé de la valeur à récupérer.
     * @return La valeur associée à la clé, ou une chaîne vide si la clé n'existe pas.
     */

    std::string get(const std::string& key) const;

};

#endif


