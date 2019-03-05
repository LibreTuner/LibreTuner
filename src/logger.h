/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LT_LOGGER_H
#define LT_LOGGER_H

#include <string>
#include <QObject>


/* Static logger */
class Logger : public QObject {
    Q_OBJECT
public:
    enum class Mode {
        Debug,
        Info,
        Warning,
        Critical,
    };

    // Returns the logger singleton
    static Logger &get();

    void log(Mode mode, const std::string &message);

    static void debug(const std::string &message) { Logger::get().log(Mode::Debug, message); }

    static void info(const std::string &message) { Logger::get().log(Mode::Info, message); }

    static void warning(const std::string &message) {
        Logger::get().log(Mode::Warning, message);
    }

    static void critical(const std::string &message) {
        Logger::get().log(Mode::Critical, message);
    }

    virtual ~Logger() = default;

    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    Logger &operator=(Logger&&) = delete;
    Logger &operator=(const Logger&) = delete;

signals:
    void appended(Mode mode, const QString &message);

private:
    static std::string modeString(Mode mode);

    Logger();
};

Q_DECLARE_METATYPE(Logger::Mode)


#endif // LT_LOGGER_H
