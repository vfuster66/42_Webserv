#pragma once

#include "HttpRequest.hpp" // Assurez-vous que cette inclusion est correcte selon votre structure de projet
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include "Logger.hpp"
#include "HttpResponse.hpp"



    		HttpRequest HttpUtils_parseRequest(const std::string& requestText);
    		void HttpUtils_parseRequestLine(const std::string& line, HttpRequest& request);
		void HttpUtils_parseHeaders(std::istringstream& stream, HttpRequest& request);
		std::string HttpUtils_determineMimeType(const std::string& filePath);
		std::string HttpUtils_loadErrorPage(int statusCode);
		void HttpUtils_parseBody(std::istringstream& stream, HttpRequest& request);
		//static std::map<std::string, std::string> HttpUtils_extractCookies(const HttpRequest& request);
    		//static void HttpUtils_setCookie(HttpResponse& response, const std::string& name, const std::string& value, int maxAge);

