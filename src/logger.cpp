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

#include "logger.h"
#include "libretuner.h"
#include <iostream>
#include <QMetaObject>

Logger::Logger() {
    qRegisterMetaType<Logger::Mode>("Mode");
}



Logger &Logger::get()
{
    static Logger logger;
    return logger;
}

void Logger::log(Logger::Mode mode, const std::string &message) {
    emit appended(mode, QString::fromStdString(message));
    std::cout << "[" << modeString(mode) << "] " << message << std::endl;
}

std::string Logger::modeString(Logger::Mode mode) {
    switch (mode) {
    case Mode::Debug:
        return "DEBUG";
    case Mode::Info:
        return "INFO";
    case Mode::Warning:
        return "WARNING";
    case Mode::Critical:
        return "CRITICAL";
    default:
        return "";
    }
}
