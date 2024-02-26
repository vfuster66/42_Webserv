#include "../includes/FileException.hpp"

FileException::FileException(const std::string& msg, int code) : AppException(msg, code)
{
}