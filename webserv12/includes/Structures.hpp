#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <vector>
#include <string>
#include <map>


std::string urlDecode(const std::string& str);

struct LocationConfig {
    std::string                             path;
    bool                                    is_regex;
    std::string                             root;
    bool                                    autoindex;
    std::vector<std::string>                allowed_methods;
    std::vector<std::string>                allowed_ips;
    std::vector<std::string>                denied_ips;
    std::vector<std::string>                index;
    std::string                             cgi_path;

    LocationConfig() : autoindex(false) {}
};

struct HttpRequest {
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> formData;

    std::string getHeader(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(key);
        if (it != headers.end())
        {
            return it->second;
        }
        return "";
    }

        void addFormData(const std::string& key, const std::string& value) {
        formData[key] = urlDecode(value);
    }
};

struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    void setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }
};

struct FilePart {
    std::string fileName;
    std::string fileContent;
};

struct ServerConfig {
    std::string                             host;
    int                                     port;
    std::vector<std::string>                server_names;
    std::string                             error_page;
    int                                     client_max_body_size;
    std::string                             root;
    std::vector<std::string>                index;
    std::vector<std::string>                allowed_ips;
    std::vector<std::string>                denied_ips;
    std::vector<LocationConfig>             locations;
};

struct CgiHandlerConfig {
    const char* extension;
    const char* handlerPath;
};

static const CgiHandlerConfig cgiConfigs[] = {
    {".php", "/usr/bin/php-cgi"},
    {".py", "/usr/bin/python"},
    {".pl", "/usr/bin/perl"},
};

#endif