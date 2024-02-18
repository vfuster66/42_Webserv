#include "HttpUtils.hpp"

HttpRequest HttpUtils_parseRequest(const std::string& requestText)
{
    HttpRequest request;
    std::istringstream stream(requestText);

    // Parse request line
    std::string requestLine;
    std::getline(stream, requestLine);
    HttpUtils_parseRequestLine(requestLine, request);

    // Parse headers
    HttpUtils_parseHeaders(stream, request);

    // Optional: Parse body if needed

    return request;
}

void HttpUtils_parseRequestLine(const std::string& line, HttpRequest& request)
{
    std::istringstream stream(line);
    stream >> request.method >> request.uri >> request.httpVersion;
}

void HttpUtils_parseHeaders(std::istringstream& stream, HttpRequest& request)
{
    std::string line;
    while (std::getline(stream, line) && line != "\r")
    {
        std::string::size_type delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos)
	{
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 2);
            // Remove possible carriage return at the end of the value
            if (!value.empty() && value[value.length() - 1] == '\r')
	    {
                value.erase(value.length() - 1);
            }
            request.headers[key] = value;
        }
    }
}

std::string HttpUtils_determineMimeType(const std::string& filePath)
{
    if (filePath.find(".html") != std::string::npos) return "text/html";
    if (filePath.find(".css") != std::string::npos) return "text/css";
    if (filePath.find(".js") != std::string::npos) return "application/javascript";
    if (filePath.find(".png") != std::string::npos) return "image/png";
    if (filePath.find(".jpg") != std::string::npos || filePath.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filePath.find(".gif") != std::string::npos) return "image/gif";
    if (filePath.find(".svg") != std::string::npos) return "image/svg+xml";
    if (filePath.find(".mp4") != std::string::npos) return "video/mp4";
    if (filePath.find(".webm") != std::string::npos) return "video/webm";
    if (filePath.find(".mp3") != std::string::npos) return "audio/mpeg";
    if (filePath.find(".wav") != std::string::npos) return "audio/wav";
    if (filePath.find(".pdf") != std::string::npos) return "application/pdf";
    if (filePath.find(".docx") != std::string::npos) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (filePath.find(".json") != std::string::npos) return "application/json";
    if (filePath.find(".xml") != std::string::npos) return "application/xml";
    return "text/plain";
}

std::string HttpUtils_loadErrorPage(int statusCode)
{
    std::ostringstream filePathStream;
    filePathStream << "www/errors/" << statusCode << ".html";
    std::string filePath = filePathStream.str();

    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (file)
    {
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    }
    else
    {
        // Fallback si le fichier d'erreur n'existe pas, en utilisant std::ostringstream pour la conversion
        std::ostringstream errorMessage;
        errorMessage << "<html><body><h1>Error " << statusCode << "</h1></body></html>";
        return errorMessage.str();
    }
}
