#include "../includes/Cookies.hpp"

Cookies::Cookies()
{
    LOG_INFO("Cookies object created.");
}

const std::map<std::string, std::string>& Cookies::getCookies() const
{
    return cookies;
}

void Cookies::parse(const std::string& header)
{
    std::istringstream stream(header);
    std::string cookie;
    LOG_INFO("Parsing cookie header: " + header);

    while (std::getline(stream, cookie, ';'))
    {
        std::size_t pos = cookie.find('=');

        if (pos != std::string::npos)
        {
            std::string name = cookie.substr(0, pos);
            std::string value = cookie.substr(pos + 1);
            cookies[name] = value;
            LOG_INFO("Parsed cookie: " + name + "=" + value);
        }
    }
}

void Cookies::extractCookiesFromRequest(const std::string& requestStr) {
    std::size_t headerStart = requestStr.find("Cookie: ");
    LOG_INFO("Extracting cookies from request");

    if (headerStart == std::string::npos) {
        LOG_INFO("No Cookie header found in request");
        return;
    }

    std::size_t headerEnd = requestStr.find("\r\n", headerStart);
    std::string cookieHeader = requestStr.substr(headerStart + 8, headerEnd - (headerStart + 8));
    LOG_INFO("Extracted Cookie header: " + cookieHeader);

    // Parse and store the cookies from the header
    // This part needs implementation based on how you parse the cookie header
    // Assuming a simple parsing logic here for demonstration
    std::istringstream iss(cookieHeader);
    std::string cookie;
    while (std::getline(iss, cookie, ';')) {
        std::size_t pos = cookie.find('=');
        if (pos != std::string::npos) {
            std::string name = cookie.substr(0, pos);
            std::string value = cookie.substr(pos + 1);
            cookies[trim(name)] = trim(value); // Assuming trim is a function to remove spaces
        }
    }
}

std::string Cookies::toString() const
{
    std::ostringstream oss;

    LOG_INFO("Building cookie string from map");

    for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        if (it != cookies.begin())
        {
            oss << "; ";
        }
        oss << it->first << "=" << it->second;

        LOG_INFO("Cookie: " + it->first + "=" + it->second);
    }

    std::string cookieString = oss.str();
    LOG_INFO("Complete cookie string: " + cookieString);

    return cookieString;
}

std::string Cookies::getValue(const std::string& name) const
{
    std::string trimmedName = trim(name);
    std::map<std::string, std::string>::const_iterator it = cookies.find(trimmedName);
    if (it != cookies.end()) {
        // Utilisation de la concaténation de string au lieu d'utiliser LOG_INFO directement
        std::string logMessage = "Cookie found: " + trimmedName + "=" + it->second;
        // Supposons que LOG_INFO soit défini ailleurs pour logger l'information
        LOG_INFO(logMessage.c_str());
        return it->second;
    } else {
        std::string logMessage = "Cookie not found: " + trimmedName;
        LOG_INFO(logMessage.c_str());
        return "";
    }
}


void Cookies::setValue(const std::string& name, const std::string& value, bool httpOnly, const std::string& path, int maxAgeSeconds)
{
    std::ostringstream oss, maxAgeOss;

    if (maxAgeSeconds > 0)
    {
        maxAgeOss << maxAgeSeconds;
    }

    oss << value << "; Path=" << path << ";";
    if (maxAgeSeconds > 0)
    {
        oss << " Max-Age=" << maxAgeOss.str() << ";";
    }
    if (httpOnly)
    {
        oss << " HttpOnly;";
    }

    cookies[name] = oss.str();

    LOG_INFO("Setting cookie: Name=" + name + ", Value=" + value + ", Path=" + path + ", Max-Age=" + (maxAgeSeconds > 0 ? maxAgeOss.str() : "N/A") + ", HttpOnly=" + (httpOnly ? "Yes" : "No"));
}

std::string Cookies::trim(const std::string& s)
{
    std::string::const_iterator start = s.begin();
    while (start != s.end() && isspace(static_cast<unsigned char>(*start))) {
        ++start;
    }

    std::string::const_reverse_iterator end = s.rbegin();
    while (end.base() != start && isspace(static_cast<unsigned char>(*end))) {
        ++end;
    }

    return std::string(start, end.base());
}
