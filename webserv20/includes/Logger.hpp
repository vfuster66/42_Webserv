#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>
#include <pthread.h>

#include "Macros.hpp"

class Logger
{

public:

    enum Level
    {
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance();
    void configure(const std::string& filename, Level fileLogLevel, Level consoleLogLevel = WARNING);
    void log(const std::string& message, Level level, const char* file, int line, const char* function);
    void cleanup();

private:

    std::ofstream       fileStream;
    Level               fileLogLevel;
    Level               consoleLogLevel;
    pthread_mutex_t     mutex;

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

