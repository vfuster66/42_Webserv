#include "../includes/ConfigException.hpp"

ConfigException::ConfigException(const std::string& msg, int code) : AppException(msg, code)
{
}
