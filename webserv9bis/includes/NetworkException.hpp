#ifndef NETWORK_EXCEPTION_HPP
#define NETWORK_EXCEPTION_HPP

#include "AppException.hpp"

class NetworkException : public AppException {
public:
    NetworkException(const std::string& msg, int code);
};

#endif
