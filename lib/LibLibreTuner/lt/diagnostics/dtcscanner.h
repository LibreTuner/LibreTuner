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

#ifndef LT_DTCSCANNER_H
#define LT_DTCSCANNER_H

#include "codes.h"
#include <memory>

namespace lt
{

class DtcScanner
{
public:
    virtual ~DtcScanner() = default;

    /* Scans the interface for codes and fills result */
    virtual DiagnosticCodes scan() = 0;
    virtual DiagnosticCodes scanPending() = 0;
};
using DtcScannerPtr = std::unique_ptr<DtcScanner>;

} // namespace lt

#endif // LT_DTCSCANNER_H
