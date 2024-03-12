#ifndef COOKIES_HPP
#define COOKIES_HPP

#include <string>
#include <map>
#include <sstream>

#include "Logger.hpp"

class Cookies
{

public:

    Cookies();

    void parse(const std::string& header);
    std::string toString() const;
    std::string getValue(const std::string& name) const;
    std::string extractCookiesFromRequest(const std::string& requestStr);
    const std::map<std::string, std::string>& getCookies() const;
    void setValue(const std::string& name, const std::string& value, bool httpOnly = false, const std::string& path = "/", const std::string& expires = "");

private:

    std::map<std::string, std::string> cookies;

};

#endif
