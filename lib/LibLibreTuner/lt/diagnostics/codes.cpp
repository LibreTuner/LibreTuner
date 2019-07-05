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

#include "codes.h"
#include <cassert>

namespace lt
{

static char firstDtc[4] = {'P', 'C', 'B', 'U'};

static char numericChar(uint8_t c)
{
    if (c <= 9)
    {
        return static_cast<char>(c) + '0';
    }
    else if (c <= 15)
    {
        return static_cast<char>(c) - 10 + 'A';
    }
    return '?';
}

std::string DiagnosticCode::codeString() const
{
    std::string res;
    res += firstDtc[(code & 0xC000) >> 14];
    res += numericChar((code & 0x3000) >> 12);
    res += numericChar((code & 0x0F00) >> 8);
    res += numericChar((code & 0x00F0) >> 4);
    res += numericChar(code & 0x000F);
    return res;
}

} // namespace lt