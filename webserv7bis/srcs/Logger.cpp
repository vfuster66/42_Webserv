#include "../includes/Logger.hpp"

const char* Logger::RESET = "\033[0m";
const char* Logger::RED = "\033[31m";
const char* Logger::YELLOW = "\033[33m";
const char* Logger::GREEN = "\033[32m";

Logger& Logger::getInstance()
{
    static Logger instance("server.log", INFO, WARNING); // Ajout d'un niveau de log pour la console
    return instance;
}

// Mise à jour du constructeur pour initialiser les deux niveaux de log
Logger::Logger(const std::string& filename, Level _fileLogLevel, Level _consoleLogLevel) 
    : fileLogLevel(_fileLogLevel), consoleLogLevel(_consoleLogLevel)
{
    pthread_mutex_init(&mutex, NULL);
    fileStream.open(filename.c_str(), std::ios::app);
    if (!fileStream.is_open())
    {
        std::cerr << "Erreur lors de l'ouverture du fichier de log : " << filename << std::endl;
    }
}

Logger::~Logger()
{
    cleanup();
}
void Logger::cleanup() {
    if (fileStream.is_open()) {
        fileStream.close();
    }
    pthread_mutex_destroy(&mutex);
}

void Logger::configure(const std::string& filename, Level _fileLogLevel, Level _consoleLogLevel)
{
    pthread_mutex_lock(&mutex);
    this->fileLogLevel = _fileLogLevel;
    this->consoleLogLevel = _consoleLogLevel;
    if (fileStream.is_open())
    {
        fileStream.close();
    }
    fileStream.open(filename.c_str(), std::ios::app);
    if (!fileStream)
    {
        std::cerr << "Erreur lors de l'ouverture du fichier de log: " << filename << std::endl;
    }
    pthread_mutex_unlock(&mutex);
}

void Logger::log(const std::string& message, Level level, const char* file, int line)
{
    pthread_mutex_lock(&mutex);

    std::ostringstream logMessage;
    const char* color = RESET;
    switch (level)
    {
        case INFO:
            color = GREEN; break;
        case WARNING:
            color = YELLOW; break;
        case ERROR:
            color = RED; break;
    }

    // Préparation du message de log
    logMessage << color << "[" << currentTime() << "][" << levelToString(level) << "]";
    if (file)
    {
        logMessage << "[" << file << ":" << line << "]";
    }
    logMessage << " " << message << RESET;

    // Log dans le fichier pour tous les niveaux
    if (fileStream.is_open() && level >= fileLogLevel) {
        fileStream << logMessage.str() << std::endl;
        fileStream.flush();
    }

    // Affichage sur le terminal pour WARNING et ERROR
    if (level >= consoleLogLevel) {
        std::cout << logMessage.str() << std::endl;
    }

    pthread_mutex_unlock(&mutex);
}

std::string Logger::levelToString(Level level)
{
    switch (level)
    {
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

std::string Logger::currentTime()
{
    std::time_t now = std::time(0);
    char buffer[20];
    std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buffer);
}


