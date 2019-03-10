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

#ifndef LT_DTCCODE_H
#define LT_DTCCODE_H

#include <string>
#include <vector>
#include <cstdint>

namespace lt {

struct DiagnosticCode {
    uint16_t code;

    std::string codeString() const;
};

using DiagnosticCodes = std::vector<DiagnosticCode>;
} // namespace lt

#endif // LT_DTCCODE_H
