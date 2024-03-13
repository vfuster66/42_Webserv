#ifndef COOKIES_HPP
#define COOKIES_HPP

#include <string>
#include <map>
#include <sstream>
#include <cctype>
#include <algorithm>

#include "Logger.hpp"

class Cookies
{

public:

    Cookies();

    void parse(const std::string& header);
    std::string toString() const;
    std::string getValue(const std::string& name) const;
    void extractCookiesFromRequest(const std::string& requestStr);
    static std::string trim(const std::string& s);
    const std::map<std::string, std::string>& getCookies() const;
    void setValue(const std::string& name, const std::string& value, bool httpOnly, const std::string& path, int maxAgeSeconds);

private:

    std::map<std::string, std::string> cookies;

};

#endif
