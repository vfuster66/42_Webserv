#Pragma once

#include "CgiHandler.hpp"
#include "StaticRequestHandler.hpp"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

struct FilePart
{
    std::string fileName;
    std::string fileContent;
};

class Upload
{
	private:

		bool isMultipartFormData(const HttpRequest& request);
    		std::string getBoundary(const std::string& contentType);
    		void parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files);
    		void saveFile(const std::string& content, const std::string& fileName);
		FilePart extractFilePart(const std::string& part);
}