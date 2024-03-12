#ifndef MACROS_HPP
#define MACROS_HPP

/*  Logger  */ 

// Definition
#define LOG_INFO(message) \
    Logger::getInstance().log((message), Logger::INFO, __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARNING(message) \
    Logger::getInstance().log((message), Logger::WARNING, __FILE__, __LINE__, __FUNCTION__)

#define LOG_ERROR(message) \
    Logger::getInstance().log((message), Logger::ERROR, __FILE__, __LINE__, __FUNCTION__) 

#endif