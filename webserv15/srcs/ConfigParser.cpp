#include "../includes/ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename, const std::string& logFile, Logger::Level logLevel)
    : filename(filename), serverCount(0)
{
    Logger& logger = Logger::getInstance();
    logger.configure(logFile, logLevel);
}

ConfigParser::~ConfigParser()
{
    servers.clear();
}

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
    LOG_INFO("Tentative d'ouverture du fichier de configuration : " + filename);

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
            LOG_INFO("Début de l'analyse d'un bloc 'server'.");
            ServerConfig serverConfig;
            parseServerBlock(configFile, serverConfig);
            servers.push_back(serverConfig);
            serverBlockCount++;
            
            std::ostringstream msg;
            msg << "Fin de l'analyse du bloc 'server' #" << serverBlockCount << ".";
            LOG_INFO(msg.str());
        }
        else
        {
            std::ostringstream msg;
            msg << "Ligne inattendue en dehors d'un bloc 'server' : " << line;
            LOG_WARNING(msg.str());
        }
    }

    if (serverBlockCount == 0)
    {
        LOG_WARNING("Aucun bloc 'server' trouvé dans le fichier de configuration.");
    }
    else
    {
        std::ostringstream msg;
        msg << "Nombre total de blocs 'server' analysés : " << serverBlockCount;
        LOG_INFO(msg.str());
    }

    configFile.close();
    LOG_INFO("Analyse du fichier de configuration terminée.");
}

void ConfigParser::parseServerBlock(std::ifstream& configFile, ServerConfig& serverConfig)
{
    std::string line;

    while (getline(configFile, line))
    {
        trim(line);

        if (line.empty())
        {
            continue;
        }

        if (line[0] == '#')
        {
            continue;
        }

        if (line == "}")
        {
            break;
        }
        else
        {
            LOG_INFO("Analyse de la configuration du serveur : " + line);
            parseKeyValue(line, serverConfig);
        }
    }

    LOG_INFO("Fin de l'analyse du bloc 'server'.");
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

    LOG_INFO("Clé traitée: " + key);

    if (key == "host")
    {
        serverConfig.host = rest;
        LOG_INFO("Host défini: " + rest);
    }
    else if (key == "port")
    {
        serverConfig.port = atoi(rest.c_str());
        LOG_INFO("Port défini: " + rest);
    }
    else if (key == "server_name")
    {
        std::istringstream namesStream(rest);
        std::string name;
        while (namesStream >> name)
        {
            serverConfig.server_names.push_back(name);
            LOG_INFO("Nom de serveur ajouté: " + name);
        }
    }
    else if (key == "error_page")
    {
        serverConfig.error_page = rest;
        LOG_INFO("Page d'erreur définie: " + rest);
    }
    else if (key == "client_max_body_size")
    {
        serverConfig.client_max_body_size = convertSizeToBytes(rest);
        LOG_INFO("Taille maximale du corps client définie: " + rest);
    }
    else if (key == "root")
    {
        serverConfig.root = rest;
        LOG_INFO("Racine définie: " + rest);
    }
    else if (key == "index")
    {
        std::istringstream indexStream(rest);
        std::string pageIndex;
        while (indexStream >> pageIndex)
        {
            serverConfig.index.push_back(pageIndex);
            LOG_INFO("Page d'index ajoutée: " + pageIndex);
        }
    }
    else if (key == "allowed_ips")
    {
        std::istringstream ipsStream(rest);
        std::string ip;
        while (ipsStream >> ip)
        {
            serverConfig.allowed_ips.push_back(ip);
            LOG_INFO("IP autorisée ajoutée: " + ip);
        }
    }
    else if (key == "denied_ips")
    {
        std::istringstream ipsStream(rest);
        std::string ip;
        while (ipsStream >> ip)
        {
            serverConfig.denied_ips.push_back(ip);
            LOG_INFO("IP refusée ajoutée: " + ip);
        }
    }
    else if (key.substr(0, 4) == "cgi_")
    {

        if (key == "cgi_bin")
        {
            serverConfig.cgi_bin = rest;
        }
        else if (key == "cgi_handler")
        {
            size_t colonPos = rest.find(':');
            if (colonPos != std::string::npos)
            {
                std::string ext = rest.substr(0, colonPos);
                std::string handlerPath = rest.substr(colonPos + 1);
                serverConfig.cgi_handlers[ext] = handlerPath;
            }
        }
        else if (key == "cgi_ext")
        {
            std::istringstream extStream(rest);
            std::string ext;
            while (getline(extStream, ext, ','))
            {
                trim(ext);
                serverConfig.cgi_ext.push_back(ext);
            }
        }
        else
        {
            std::vector<std::string>::iterator it = std::find(serverConfig.cgi_ext.begin(), serverConfig.cgi_ext.end(), key);
            if (it != serverConfig.cgi_ext.end())
            {
                serverConfig.cgi_handlers[key] = rest;
            }
            else
            {
                LOG_WARNING("Clé non reconnue ou non prise en charge: " + key);
            }
        }
    }
    else if (std::find(serverConfig.cgi_ext.begin(), serverConfig.cgi_ext.end(), key) != serverConfig.cgi_ext.end()) {

        serverConfig.cgi_handlers[key] = rest;
    }
    else if (key == "allowed_methods")
    {
        std::istringstream methodsStream(rest);
        std::string method;
        while (getline(methodsStream, method, ','))
        {
            trim(method);
            serverConfig.allowed_methods.push_back(method);
            LOG_INFO("Méthode autorisée ajoutée: " + method);
        }
    }
    else if (key == "denied_methods")
    {
        std::istringstream methodsStream(rest);
        std::string method;
        while (getline(methodsStream, method, ','))
        {
            trim(method);
            serverConfig.denied_methods.push_back(method);
            LOG_INFO("Méthode refusée ajoutée: " + method);
        }
    }
    else
    {
        LOG_WARNING("Clé non reconnue ou non prise en charge: " + key);
    }
}

// Utils
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

std::string ConfigParser::cleanValue(const std::string& value)
{
    std::string cleanedValue = value;
    cleanedValue.erase(std::remove(cleanedValue.begin(), cleanedValue.end(), ';'), cleanedValue.end());
    return cleanedValue;
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
        LOG_INFO("La chaîne de taille est vide, renvoie 0.");
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
        LOG_WARNING("Aucune partie numérique trouvée, renvoie 0.");
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
                break;
            case 'm':
                finalSize = static_cast<long>(size) * 1024 * 1024;
                break;
            case 'g':
                finalSize = static_cast<long>(size) * 1024 * 1024 * 1024;
                break;
            default:
                finalSize = size;
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
