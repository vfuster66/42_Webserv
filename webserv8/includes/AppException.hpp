#ifndef APP_EXCEPTION_HPP
#define APP_EXCEPTION_HPP

#include <exception>
#include <string>
#include <sstream>

#include "Logger.hpp"

#define THROW_APP_EXCEPTION(msg, code) throw AppException(msg, code, __FILE__, __LINE__)

class AppException : public std::exception
{

protected:
    std::string message;
    int errorCode;
    std::string file;
    int line;

public:
    AppException(const std::string& msg, int code, const char* file = NULL, int line = 0);

    virtual ~AppException() throw() {}

    virtual const char* what() const throw();

    int getCode() const;

};

#endif


