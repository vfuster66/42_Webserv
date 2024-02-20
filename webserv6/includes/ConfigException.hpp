#ifndef CONFIG_EXCEPTION_HPP
#define CONFIG_EXCEPTION_HPP

#include "AppException.hpp"

class ConfigException : public AppException
{
public:
    ConfigException(const std::string& msg, int code);
};

#endif
