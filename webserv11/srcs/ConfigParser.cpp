#include "../includes/ConfigParser.hpp"

// Constructeur
ConfigParser::ConfigParser(const std::string& filename, const std::string& logFile, Logger::Level logLevel)
    : filename(filename), serverCount(0)
{
    Logger& logger = Logger::getInstance();
    logger.configure(logFile, logLevel);

    LOG_INFO("ConfigParser instance created for file: " + filename);
}

// Destructeur
ConfigParser::~ConfigParser()
{
    servers.clear();
}

// Opérateur d'assignation
ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this != &other)
    {
        filename = other.filename;
        servers = other.servers;
        serverCount = other.serverCount;
        iss.str(other.iss.str());
    }
    return *this;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
    return servers;
}

void ConfigParser::parse()
{
    LOG_INFO("parse -> Tentative d'ouverture du fichier de configuration : " + filename);

    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open())
    {
        throw std::runtime_error("Could not open config file.");
    }

    std::string line;
    int serverBlockCount = 0;
    while (getline(configFile, line))
    {
        trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        if (line == "server {")
        {
            LOG_INFO("parse -> Début de l'analyse d'un bloc 'server'.");
            ServerConfig serverConfig;
            parseServerBlock(configFile, serverConfig);
            servers.push_back(serverConfig);
            serverBlockCount++;
            
            std::ostringstream msg;
            msg << "parse -> Fin de l'analyse du bloc 'server' #" << serverBlockCount << ".";
            LOG_INFO(msg.str());
        }
        else
        {
            std::ostringstream msg;
            msg << "parse -> Ligne inattendue en dehors d'un bloc 'server' : " << line;
            LOG_WARNING(msg.str());
        }
    }

    if (serverBlockCount == 0)
    {
        LOG_WARNING("parse -> Aucun bloc 'server' trouvé dans le fichier de configuration.");
    }
    else
    {
        std::ostringstream msg;
        msg << "parse -> Nombre total de blocs 'server' analysés : " << serverBlockCount;
        LOG_INFO(msg.str());
    }

    configFile.close();
    LOG_INFO("parse -> Analyse du fichier de configuration terminée.");
}

// Parsing bloc server
void ConfigParser::parseServerBlock(std::ifstream& configFile, ServerConfig& serverConfig)
{
    LOG_INFO("parseServerBlock -> Début de l'analyse d'un bloc 'server'.");
    std::string line;
    int locationCount = 0;

    while (getline(configFile, line))
    {
        trim(line);

        if (line.empty())
        {
            LOG_INFO("parseServerBlock -> Ignorer une ligne vide.");
            continue;
        }

        if (line[0] == '#')
        {
            LOG_INFO("parseServerBlock -> Ignorer un commentaire : " + line);
            continue;
        }

        if (line.find("location") == 0)
        {
            locationCount++;
            std::ostringstream msg;
            msg << "parseServerBlock -> Début de l'analyse d'un bloc 'location' #" << locationCount << ".";
            LOG_INFO(msg.str());
            parseLocationBlock(configFile, serverConfig);
            msg.str("");
            msg.clear();
            msg << "parseServerBlock -> Fin de l'analyse du bloc 'location' #" << locationCount << ".";
            LOG_INFO(msg.str());
        }
        else if (line == "}")
        {
            LOG_INFO("parseServerBlock -> Fin du bloc 'server' détectée.");
            break;
        }
        else
        {
            LOG_INFO("parseServerBlock -> Analyse de la configuration du serveur : " + line);
            parseKeyValue(line, serverConfig);
        }
    }

    std::ostringstream finalMsg;
    finalMsg << "parseServerBlock -> Fin de l'analyse du bloc 'server'. " << locationCount << " blocs 'location' traités.";
    LOG_INFO(finalMsg.str());
}

void ConfigParser::parseKeyValue(const std::string& line, ServerConfig& serverConfig)
{
    std::istringstream iss(line);
    std::string key, rest;
    getline(iss, key, ':');
    getline(iss, rest);
    trim(key);
    trim(rest);
    removeQuotes(rest);

    LOG_INFO("parseKeyValue -> Clé traitée: " + key);

    if (key == "host")
    {
        serverConfig.host = rest;
        LOG_INFO("parseKeyValue -> Host défini: " + rest);
    }
    else if (key == "port")
    {
        serverConfig.port = atoi(rest.c_str());
        LOG_INFO("parseKeyValue -> Port défini: " + rest);
    }
    else if (key == "server_name")
    {
        std::istringstream namesStream(rest);
        std::string name;
        while (namesStream >> name)
        {
            serverConfig.server_names.push_back(name);
            LOG_INFO("parseKeyValue -> Nom de serveur ajouté: " + name);
        }
    }
    else if (key == "error_page")
    {
        serverConfig.error_page = rest;
        LOG_INFO("parseKeyValue -> Page d'erreur définie: " + rest);
    }
    else if (key == "client_max_body_size")
    {
        serverConfig.client_max_body_size = convertSizeToBytes(rest);
        LOG_INFO("parseKeyValue -> Taille maximale du corps client définie: " + rest);
    }
    else if (key == "root")
    {
        serverConfig.root = rest;
        LOG_INFO("parseKeyValue -> Racine définie: " + rest);
    }
    else if (key == "index")
    {
        std::istringstream indexStream(rest);
        std::string pageIndex;
        while (indexStream >> pageIndex)
        {
            serverConfig.index.push_back(pageIndex);
            LOG_INFO("parseKeyValue -> Page d'index ajoutée: " + pageIndex);
        }
    }
    else if (key == "allowed_ips")
    {
        std::istringstream ipsStream(rest);
        std::string ip;
        while (ipsStream >> ip)
        {
            serverConfig.allowed_ips.push_back(ip);
            LOG_INFO("parseKeyValue -> IP autorisée ajoutée: " + ip);
        }
    }
    else if (key == "denied_ips")
    {
        std::istringstream ipsStream(rest);
        std::string ip;
        while (ipsStream >> ip)
        {
            serverConfig.denied_ips.push_back(ip);
            LOG_INFO("parseKeyValue -> IP refusée ajoutée: " + ip);
        }
    }
    else
    {
        LOG_WARNING("parseKeyValue -> Clé non reconnue ou non prise en charge: " + key);
    }
}

// Parsing bloc location
void ConfigParser::parseLocationBlock(std::ifstream& configFile, ServerConfig& serverConfig) {
    LOG_INFO("parseLocationBlock -> Début de l'analyse d'un bloc 'location'.");
    std::string line;

    while (std::getline(configFile, line)) {
        trim(line);

        if (line.empty() || line[0] == '#') {
            LOG_INFO(std::string("parseLocationBlock -> Ignoré: ") + (line.empty() ? "ligne vide" : "commentaire"));
            continue;
        }

        if (line == ">") {
            // Log pour la fin du bloc 'location'
            LOG_INFO("parseLocationBlock -> Fin du bloc 'location' détectée avec '>'.");
            break;
        }

        if (line[0] == '/' || line[0] == '~') {
            LOG_INFO("parseLocationBlock -> Traitement d'une nouvelle règle de location: " + line);
            LocationConfig locationConfig;
            std::istringstream lineStream(line);
            std::string path;
            getline(lineStream, path, ' ');
            locationConfig.path = path;

            if (path[0] == '~') {
                locationConfig.is_regex = true;
                LOG_INFO("parseLocationBlock -> Chemin avec expression régulière détecté: " + path);
            } else {
                LOG_INFO("parseLocationBlock -> Chemin normal détecté: " + path);
            }

            parseLocationLine(lineStream, locationConfig);

            serverConfig.locations.push_back(locationConfig);
            LOG_INFO("parseLocationBlock -> Configuration de location ajoutée avec succès.");
        } else {
            LOG_WARNING("parseLocationBlock -> Directive non reconnue: " + line);
        }
    }

    LOG_INFO("parseLocationBlock -> Analyse du bloc 'location' terminée avec succès.");
}

void ConfigParser::parseLocationLine(std::istringstream& lineStream, LocationConfig& locationConfig) {
    std::string config;
    while (lineStream >> config) {
        if (config == "autoindex") {
            std::string value;
            lineStream >> value;
            locationConfig.autoindex = (value == "on");
            LOG_INFO("parseLocationBlock -> Autoindex défini à: " + std::string(locationConfig.autoindex ? "on" : "off"));
        } else if (config == "GET" || config == "POST" || config == "DELETE") {
            locationConfig.allowed_methods.push_back(config);
            LOG_INFO("parseLocationBlock -> Méthode autorisée ajoutée: " + config);
        } else {
            if (config == "cgi") {
                lineStream >> locationConfig.cgi_path;
                LOG_INFO("parseLocationBlock -> Chemin CGI spécifié: " + locationConfig.cgi_path);
            } else {
                locationConfig.root = config;
                LOG_INFO("parseLocationBlock -> Racine définie pour la location: " + locationConfig.root);
                break;
            }
        }
    }
}

// Utils
std::string ConfigParser::get(const std::string& key) const
{
    if (servers.empty())
    {
        LOG_WARNING("ConfigParser::get -> Tentative d'accès à la configuration alors que les serveurs sont vides.");
        return "";
    }

    const ServerConfig& serverConfig = servers[0];
    std::ostringstream convert;

    if (key == "host")
    {
        LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'host': " + serverConfig.host);
        return serverConfig.host;
    }
    else if (key == "port")
    {
        convert << serverConfig.port;
        LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'port': " + convert.str());
        return convert.str();
    }
    else if (key == "server_name")
    {
        if (!serverConfig.server_names.empty())
        {
            LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'server_name': " + serverConfig.server_names[0]);
            return serverConfig.server_names[0];
        }
        else
        {
            LOG_WARNING("ConfigParser::get -> 'server_name' demandé mais aucun nom de serveur configuré.");
        }
    }
    else if (key == "error_page")
    {
        LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'error_page': " + serverConfig.error_page);
        return serverConfig.error_page;
    }
    else if (key == "client_max_body_size")
    {
        convert.str("");
        convert << serverConfig.client_max_body_size;
        LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'client_max_body_size': " + convert.str());
        return convert.str();
    }
    else if (key == "root")
    {
        LOG_INFO("ConfigParser::get -> Accès à la configuration pour 'root': " + serverConfig.root);
        return serverConfig.root;
    }

    LOG_WARNING("ConfigParser::get -> Clé de configuration non reconnue ou absente: " + key);
    return "";
}


void ConfigParser::resetISS(std::istringstream& iss, const std::string& newStr)
{
    iss.clear();
    iss.str(newStr);
}

void ConfigParser::trim(std::string& str) const
{
    std::string::size_type first = str.find_first_not_of(" \t\n\r\f\v");
    std::string::size_type last = str.find_last_not_of(" \t\n\r\f\v");

    if (first == std::string::npos || last == std::string::npos)
    {
        str.clear();
    }
    else
    {
        str = str.substr(first, (last - first + 1));
    }
}

std::string ConfigParser::removeQuotes(const std::string& input) const
{
    std::string result = input;
    if (!result.empty() && (result[0] == '"' || result[0] == '\''))
        result.erase(0, 1);

    if (!result.empty() && (result[result.size() - 1] == '"' || result[result.size() - 1] == '\''))
        result.erase(result.size() - 1);

    return result;
}

int ConfigParser::convertSizeToBytes(const std::string& sizeStr) const
{
    if (sizeStr.empty())
    {
        LOG_INFO("convertSizeToBytes -> La chaîne de taille est vide, renvoie 0.");
        return 0;
    }

    std::string numPart;
    size_t idx = 0;
    while (idx < sizeStr.size() && isdigit(sizeStr[idx]))
    {
        numPart += sizeStr[idx];
        ++idx;
    }

    if (numPart.empty())
    {
        LOG_WARNING("convertSizeToBytes -> Aucune partie numérique trouvée, renvoie 0.");
        return 0;
    }

    int size = atoi(numPart.c_str());
    long finalSize;
    if (idx < sizeStr.size())
    {
        char lastChar = tolower(sizeStr[idx]);

        switch (lastChar)
        {
            case 'k':
                finalSize = static_cast<long>(size) * 1024;
                LOG_INFO("convertSizeToBytes -> Conversion avec 'K' effectuée.");
                break;
            case 'm':
                finalSize = static_cast<long>(size) * 1024 * 1024;
                LOG_INFO("convertSizeToBytes -> Conversion avec 'M' effectuée.");
                break;
            case 'g':
                finalSize = static_cast<long>(size) * 1024 * 1024 * 1024;
                LOG_INFO("convertSizeToBytes -> Conversion avec 'G' effectuée.");
                break;
            default:
                finalSize = size;
                LOG_INFO("convertSizeToBytes -> Aucun multiplicateur trouvé, utilise la taille telle quelle.");
                break;
        }
    }
    else
    {
        finalSize = size;
        LOG_INFO("convertSizeToBytes -> Aucun multiplicateur trouvé, utilise la taille telle quelle.");
    }

    if (finalSize > INT_MAX)
    {
        LOG_WARNING("convertSizeToBytes -> La taille convertie dépasse INT_MAX, renvoie INT_MAX.");
        return INT_MAX;
    }
    else
    {
        return static_cast<int>(finalSize);
    }
}
