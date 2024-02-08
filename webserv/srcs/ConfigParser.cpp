#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : filename(filename)
{
}

ConfigParser::ConfigParser(const ConfigParser& other) : filename(other.filename), configMap(other.configMap)
{
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this != &other)
    {
        filename = other.filename;
        configMap = other.configMap;
    }
    return *this;
}

ConfigParser::~ConfigParser() 
{
}

void ConfigParser::parse()
{
    std::ifstream configFile(filename.c_str());

    if (!configFile.is_open())
    {
        throw std::runtime_error("Cannot open config file: " + filename);
    }

    std::string line;

    while (std::getline(configFile, line))
    {
        std::istringstream is_line(line);
        std::string key;

        if (std::getline(is_line, key, '='))
        {
            std::string value;

            if (std::getline(is_line, value))
            {
                configMap[trim(key)] = trim(value);
            }
        }
    }
}

std::string ConfigParser::get(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = configMap.find(key);

    if (it != configMap.end())
    {
        return it->second;
    }
    else
    {
        return "";
    }
}

std::string ConfigParser::trim(const std::string& str) const
{
    size_t first = str.find_first_not_of(' ');

    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(' ');

    return str.substr(first, (last - first + 1));
}

