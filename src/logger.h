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
