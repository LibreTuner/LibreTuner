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

#ifndef DTCDESCRIPTIONS_H
#define DTCDESCRIPTIONS_H

#include <utility>
#include <string>
#include <unordered_map>

class DtcDescriptions
{
public:
    DtcDescriptions() = default;
    DtcDescriptions(DtcDescriptions&&) = delete;
    DtcDescriptions(const DtcDescriptions&&) = delete;
    DtcDescriptions &operator=(const DtcDescriptions&) = delete;

    /* Load DTC description file */
    void load();

    /* Returns (true, desc) where desc is the description for code or (false, "") if no description
     * is available */
    std::pair<bool, std::string> get(const std::string &code) const;

private:
    // <code, description> map
    std::unordered_map<std::string, std::string> descriptions_;
};

#endif // DTCDESCRIPTIONS_H
