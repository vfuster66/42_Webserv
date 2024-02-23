#include "../includes/Upload.hpp"

bool Upload::isMultipartFormData(const HttpRequest& request)
{
    std::string contentType;
    std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        contentType = it->second;
        LOG_INFO("isMultipartFormData -> Content-Type trouvé : " + contentType);
    }
    else
    {
        LOG_WARNING("isMultipartFormData -> Aucun Content-Type trouvé dans les en-têtes de la requête.");
    }

    bool isMultipart = contentType.find("multipart/form-data") != std::string::npos;

    if (isMultipart)
    {
        LOG_INFO("isMultipartFormData -> La requête est de type multipart/form-data.");
    }
    else
    {
        LOG_INFO("isMultipartFormData -> La requête n'est pas de type multipart/form-data.");
    }

    return isMultipart;
}


std::string Upload::getBoundary(const std::string& contentType)
{
    LOG_INFO("getBoundary -> Début de getBoundary avec contentType: " + contentType);

    std::string::size_type pos = contentType.find("boundary=");
    if (pos != std::string::npos)
    {
        std::string boundary = contentType.substr(pos + 9);
        LOG_INFO("getBoundary -> Boundary trouvé: " + boundary);
        return boundary;
    }

    LOG_WARNING("getBoundary -> Aucun boundary trouvé dans contentType");
    return "";
}

void Upload::parseMultipartFormData(const std::string& body, const std::string& boundary, std::vector<FilePart>& files)
{
    LOG_INFO("parseMultipartFormData -> Début de parseMultipartFormData");

    std::string delimiter = "--" + boundary;
    std::string endDelimiter = delimiter + "--";
    std::size_t pos = 0, endPos = 0;

    while ((pos = body.find(delimiter, pos)) != std::string::npos)
    {
        endPos = body.find(delimiter, pos + delimiter.length());

        if (endPos == std::string::npos)
            break;

        std::string part = body.substr(pos + delimiter.length(), endPos - pos - delimiter.length());
        FilePart filePart = extractFilePart(part);

        if (!filePart.fileName.empty())
        {
            LOG_INFO("parseMultipartFormData -> Fichier extrait: " + filePart.fileName);
            files.push_back(filePart);
        }

        pos = endPos + delimiter.length();
    }

    LOG_INFO("parseMultipartFormData -> Fin de parseMultipartFormData");
}

void Upload::saveFile(const std::string& content, const std::string& fileName)
{
    std::string filePath = "/home/vfuster-/42-Webserv/webserv4/uploads/" + fileName;

    std::ofstream file(filePath.c_str(), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write(content.c_str(), content.size());
        file.close();
        LOG_INFO("saveFile -> Fichier sauvegardé avec succès : " + filePath);
    }
    else
    {
        LOG_ERROR("saveFile -> Erreur lors de l'ouverture du fichier pour écriture : " + filePath);
    }
}

FilePart Upload::extractFilePart(const std::string& part)
{
    FilePart filePart;
    std::istringstream partStream(part);
    std::string line;
    std::string contentDisposition;
    bool contentStart = false;

    while (std::getline(partStream, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }

        if (line.empty())
        {
            contentStart = true;
            LOG_INFO("extractFilePart -> Début du contenu du fichier détecté.");
            continue;
        }

        if (!contentStart)
        {
            if (line.find("Content-Disposition:") != std::string::npos)
            {
                contentDisposition = line;
                LOG_INFO("extractFilePart -> Content-Disposition trouvé : " + line);
                std::size_t filenamePos = line.find("filename=\"");
                if (filenamePos != std::string::npos)
                {
                    filenamePos += 10;
                    std::size_t filenameEnd = line.find("\"", filenamePos);
                    if (filenameEnd != std::string::npos)
                    {
                        filePart.fileName = line.substr(filenamePos, filenameEnd - filenamePos);
                        LOG_INFO("extractFilePart -> Nom du fichier extrait : " + filePart.fileName);
                    }
                }
            }
        }
        else
        {
            filePart.fileContent += line + "\n";
        }
    }

    if (!filePart.fileContent.empty())
    {
        filePart.fileContent.erase(filePart.fileContent.size() - 1);
    }

    LOG_INFO("extractFilePart -> Extraction du fichier terminée. Nom du fichier : " + filePart.fileName);
    return filePart;
}
