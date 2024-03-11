#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Logger.hpp"
#include "Structures.hpp"

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
#include <sys/stat.h>

class ConfigParser
{

private:
    std::string                 filename;
    std::vector<ServerConfig>   servers;    
    int                         serverCount;
    std::istringstream          iss;

    ConfigParser(const ConfigParser& other);

    void        resetISS(std::istringstream& iss, const std::string& newStr);
    void        trim(std::string& str) const;
    std::string removeQuotes(const std::string& input) const;
    std::string cleanValue(const std::string& value);
    int         convertSizeToBytes(const std::string& sizeStr) const;

    void        parseServerBlock(std::ifstream& configFile, ServerConfig& serverConfig);
    void        parseKeyValue(const std::string& line, ServerConfig& serverConfig);

public:

    explicit ConfigParser(const std::string& filename, const std::string& logFile, Logger::Level logLevel);
    ~ConfigParser();

    ConfigParser& operator=(const ConfigParser& other);

    void parse();
    const std::vector<ServerConfig>& getServers() const;

};

#endif

