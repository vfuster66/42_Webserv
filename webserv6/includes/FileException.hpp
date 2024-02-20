#ifndef FILE_EXCEPTION_HPP
#define FILE_EXCEPTION_HPP

#include "AppException.hpp"

class FileException : public AppException {
public:
    FileException(const std::string& msg, int code);
};

#endif
