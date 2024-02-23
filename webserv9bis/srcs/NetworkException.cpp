#include "../includes/NetworkException.hpp"

NetworkException::NetworkException(const std::string& msg, int code) : AppException(msg, code)
{
}