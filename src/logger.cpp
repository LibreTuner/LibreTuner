#include "logger.h"
#include <iostream>

Logger::Logger()
= default;

void Logger::log(Logger::Mode mode, const std::string &message)
{
    std::cout << "[" << modeString(mode) << "] " << message << std::endl;
}

std::string Logger::modeString(Logger::Mode mode)
{
    switch (mode) {
    case Logger::Mode::Debug:
        return "DEBUG";
    case Logger::Mode::Info:
        return "INFO";
    case Logger::Mode::Warning:
        return "WARNING";
    case Logger::Mode::Critical:
        return "CRITICAL";
    default:
        return "";
    }
}
