#include "../includes/AppException.hpp"

AppException::AppException(const std::string& msg, int code, const char* _file, int _line)
    : message(msg), errorCode(code), line(_line)
{
    if (_file != NULL)
    {
        this->file = _file;
    }
    else
    {
        this->file = "";
    }

    std::ostringstream oss;
    oss << "Exception: " << message << " Code: " << errorCode;
    if (!this->file.empty())
    {
        oss << " in " << this->file << " at line " << this->line;
    }

    LOG_ERROR(oss.str());
}

const char* AppException::what() const throw()
{
    return message.c_str();
}

int AppException::getCode() const
{
    return errorCode;
}



