#include "../includes/Cookies.hpp"

Cookies::Cookies()
{
}

const std::map<std::string, std::string>& Cookies::getCookies() const {
    return cookies;
}

void Cookies::parse(const std::string& header)
{
    std::istringstream stream(header);
    std::string cookie;

    while (std::getline(stream, cookie, ';'))
    {
        std::size_t pos = cookie.find('=');

        if (pos != std::string::npos)
        {
            std::string name = cookie.substr(0, pos);
            std::string value = cookie.substr(pos + 1);
            cookies[name] = value;
        }
    }
    LOG_INFO("Parsed cookies from header");
}

std::string Cookies::extractCookiesFromRequest(const std::string& requestStr)
{
    std::size_t headerStart = requestStr.find("Cookie: ");

    if (headerStart == std::string::npos)
    {
        return "";
    }

    std::size_t headerEnd = requestStr.find("\r\n", headerStart);
    std::string cookieHeader = requestStr.substr(headerStart + 8, headerEnd - (headerStart + 8));
    LOG_INFO("Extracted Cookie header: " + cookieHeader);

    return cookieHeader;
}

std::string Cookies::toString() const
{
    std::ostringstream oss;
    for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        if (it != cookies.begin())
        {
            oss << "; ";
        }
        oss << it->first << "=" << it->second;
    }
    return oss.str();
}

std::string Cookies::getValue(const std::string& name) const
{
    std::map<std::string, std::string>::const_iterator it = cookies.find(name);
    if (it != cookies.end())
    {
        return it->second;
    }
    return "";
}

void Cookies::setValue(const std::string& name, const std::string& value, bool httpOnly, const std::string& path, const std::string& expires) {
    std::ostringstream oss;
    oss << name << "=" << value << "; Path=" << path << ";";

    if (!expires.empty()) {
        oss << " Expires=" << expires << ";";
    }

    if (httpOnly) {
        oss << " HttpOnly;";
    }

    cookies[name] = oss.str();
}
