#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <pthread.h>

#define LOG_INFO(message) \
    Logger::getInstance().log((message), Logger::INFO, __FILE__, __LINE__)

#define LOG_WARNING(message) \
    Logger::getInstance().log((message), Logger::WARNING, __FILE__, __LINE__)

#define LOG_ERROR(message) \
    Logger::getInstance().log((message), Logger::ERROR, __FILE__, __LINE__)

class Logger {
public:
    enum Level {
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance();

    void configure(const std::string& filename, Level fileLogLevel, Level consoleLogLevel = WARNING);

    void log(const std::string& message, Level level, const char* file = NULL, int line = 0);

    void cleanup();

private:
    std::ofstream fileStream;
    Level fileLogLevel;
    Level consoleLogLevel;
    pthread_mutex_t mutex;

    Logger(const std::string& filename, Level fileLogLevel, Level consoleLogLevel = WARNING);
    ~Logger();

    Logger(const Logger&);
    Logger& operator=(const Logger&);

    std::string levelToString(Level level);
    std::string currentTime();

    static const char* RESET;
    static const char* RED;
    static const char* YELLOW;
    static const char* GREEN;
};

#endif

