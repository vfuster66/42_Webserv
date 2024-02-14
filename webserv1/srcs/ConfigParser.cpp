#include "../includes/ConfigParser.hpp"

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
        std::string trimmedLine = trim(line);

        if (trimmedLine.empty() || trimmedLine[0] == '#' || trimmedLine.substr(0, 2) == "//") {
            continue;
        }

        std::istringstream is_line(trimmedLine);
        std::string key;

        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value))
            {
                value = trim(removeQuotes(value));
                configMap[trim(key)] = value;
            }
            else
            {
                throw std::runtime_error("Invalid line format: " + trimmedLine);
            }
        }
        else
        {
            throw std::runtime_error("Invalid line format, '=' not found: " + trimmedLine);
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

std::string ConfigParser::removeQuotes(const std::string& input) const
{
    std::string result = input;
    if (!result.empty() && (result[0] == '"' || result[0] == '\'') && result[0] == result[result.size() - 1])
    {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

int ConfigParser::getInt(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = configMap.find(key);
    if (it != configMap.end()) {
        const char* str = it->second.c_str();
        char* end;
        long value = std::strtol(str, &end, 10);
        if (end == str) {
            throw std::runtime_error("Conversion error: Key '" + key + "' cannot be converted to int.");
        }
        return static_cast<int>(value);
    } else {
        throw std::runtime_error("Key not found: " + key);
    }
}

float ConfigParser::getFloat(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = configMap.find(key);
    if (it != configMap.end()) {
        const char* str = it->second.c_str();
        char* end;
        float value = static_cast<float>(std::strtod(str, &end));
        if (end == str) {
            throw std::runtime_error("Conversion error: Key '" + key + "' cannot be converted to float.");
        }
        return value;
    } else {
        throw std::runtime_error("Key not found: " + key);
    }
}

bool ConfigParser::getBool(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = configMap.find(key);
    if (it != configMap.end())
    {
        std::string value = trim(removeQuotes(it->second));
        for (std::string::iterator i = value.begin(); i != value.end(); ++i)
        {
            *i = std::tolower(*i);
        }

        if (value == "true" || value == "1")
        {
            return true;
        }
        else if (value == "false" || value == "0")
        {
            return false;
        }
        else
        {
            throw std::runtime_error("Conversion error: Key '" + key + "' cannot be converted to bool.");
        }
    }
    else 
    {
        throw std::runtime_error("Key not found: " + key);
    }
}

    return str.substr(first, (last - first + 1));
}

