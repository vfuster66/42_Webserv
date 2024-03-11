#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime>

class HttpRequest;
class HttpResponse;
class FilePart;

std::string urlDecode(const std::string& str);

struct HttpRequest
{

    std::string                         method;
    std::string                         uri;
    std::string                         httpVersion;
    std::string                         body;
    std::string                         queryString;
    std::map<std::string, std::string>  headers;
    std::map<std::string, std::string>  formData;

    std::string getHeader(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = headers.find(key);
        if (it != headers.end())
        {
            return it->second;
        }
        return "";
    }

    void addFormData(const std::string& key, const std::string& value)
    {
        formData[key] = urlDecode(value);
    }

};

struct HttpResponse
{

    std::string                         httpVersion;
    std::string                         body;
    std::string                         statusMessage;
    int                                 statusCode;
    std::map<std::string, std::string>  headers;

    void setHeader(const std::string& key, const std::string& value)
    {
        headers[key] = value;
    }

};

struct FilePart
{

    std::string                         fileName;
    std::string                         fileContent;

};

struct ServerConfig
{

    std::string                         host;
    std::string                         error_page;
    std::string                         root;
    std::string                         cgi_bin;
    bool                                generate_index_html;
    bool                                directory_listing;
    int                                 port;
    int                                 client_max_body_size;
    std::vector<std::string>            server_names;
    std::vector<std::string>            index;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            denied_methods;
    std::vector<std::string>            allowed_ips;
    std::vector<std::string>            denied_ips;
    std::vector<std::string>            cgi_ext;
    std::map<std::string, std::string>  cgi_handlers;
    std::map<std::string, std::string>  redirections;
    std::map<std::string, std::string>  route_specific_root;

};

struct Session
{

    std::string                         sessionId;
    std::time_t                         lastActivity;
    std::map<std::string, std::string> userData;

    Session() : lastActivity(std::time(0))
    {
    }

    Session(const std::string& id) : sessionId(id), lastActivity(std::time(0))
    {
    }

};

#endif

