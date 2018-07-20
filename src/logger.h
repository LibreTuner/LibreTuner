#ifndef LOGGER_H
#define LOGGER_H

#include <string>

/* Static logger */
class Logger
{
public:
    enum class Mode {
        Debug,
        Info,
        Warning,
        Critical,
    };

    Logger();

    static void log(Mode mode, const std::string &message);

    static void debug(const std::string &message) {
        log(Mode::Debug, message);
    }

    static void info(const std::string &message) {
        log(Mode::Info, message);
    }

    static void warning(const std::string &message) {
        log(Mode::Warning, message);
    }

    static void critical(const std::string &message) {
        log(Mode::Critical, message);
    }

private:
    static std::string modeString(Mode mode);
};

#endif // LOGGER_H
