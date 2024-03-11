#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <vector>
#include <string>
#include <map>
#include <ctime>



std::string urlDecode(const std::string& str);

struct HttpRequest {
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    std::map<std::string, std::string> formData;
    std::string queryString;

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

    bool                                    generate_index_html;
    bool                                    directory_listing;
    std::map<std::string, std::string>      redirection;
    std::string                             host;
    int                                     port;
    std::vector<std::string>                server_names;
    std::string                             error_page;
    int                                     client_max_body_size;
    std::string                             root;
    std::vector<std::string>                index;
    std::vector<std::string>                allowed_methods;
    std::vector<std::string>                denied_methods;
    std::vector<std::string>                allowed_ips;
    std::vector<std::string>                denied_ips;
    std::string                             cgi_bin;
    std::map<std::string, std::string>      cgi_handlers;
    std::vector<std::string>                cgi_ext;
};

struct Session {
    std::string sessionId;
    std::time_t lastActivity;
    std::map<std::string, std::string> userData;

    Session() : lastActivity(std::time(0))
    {
    }

    Session(const std::string& id) : sessionId(id), lastActivity(std::time(0))
    {
    }
};

#endif