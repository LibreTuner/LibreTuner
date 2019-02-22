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

#ifndef DIAGNOSTICSINTERFACE_H
#define DIAGNOSTICSINTERFACE_H

#include "scanresult.h"
#include <memory>

namespace uds {
class Protocol;
}

class DiagnosticsInterface {
public:
    virtual ~DiagnosticsInterface() = default;

    /* Scans the interface for codes and fills result */
    virtual void scan(ScanResult &result) = 0;
    virtual void scanPending(ScanResult &result) = 0;
};



class UdsDiagnosticInterface : public DiagnosticsInterface {
public:
    explicit UdsDiagnosticInterface(std::unique_ptr<uds::Protocol> &&uds);


    virtual void scan(ScanResult &result) override;
    virtual void scanPending(ScanResult &result) override;

private:
    std::unique_ptr<uds::Protocol> uds_;

    void scanPid(ScanResult &result, uint8_t pid = 0x3);
};

#endif // DIAGNOSTICSINTERFACE_H
