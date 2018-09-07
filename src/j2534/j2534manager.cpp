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

#include "j2534manager.h"
#include "interface.h"
#include "interfacemanager.h"
#include "j2534.h"
#include "logger.h"
#include <Windows.h>

J2534Manager::J2534Manager() {}

J2534Manager &J2534Manager::get() {
    static J2534Manager manager;
    return manager;
}

void J2534Manager::init() { load_interfaces(); }

void J2534Manager::load_interfaces() {
    // This function need optimized. Clearing the interfaces and loading them is
    // bad because:
    // 1. Unecessarily loading the DLL twice
    // 2. Existing devices keep a reference to the J2534 interface, so it will
    // not be destructed
    //    and two copies will exist
    interfaces_.clear();
    InterfaceManager::get().clearAuto();
    // Ugly C code incoming

    // Search HKEY_LOCAL_MACHINE\SOFTWARE\PassThruSupport.04.04 for connected
    // interfaces
    HKEY hKeySoftware;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ,
                     &hKeySoftware) != ERROR_SUCCESS) {
        throw std::runtime_error(
            "Could not open HKEY_LOCAL_MACHINE\\Software for reading");
    }

    LSTATUS res;
    /*HKEY hKeyWow;
    if ((res = RegOpenKeyEx(hKeySoftware, "WOW6432Node", 0, KEY_READ, &hKeyWow))
    != ERROR_SUCCESS) { RegCloseKey(hKeySoftware); if (res ==
    ERROR_FILE_NOT_FOUND) {
            // If this entry does not exist, then no PassThru interfaces are
    installed return;
        }

        throw std::runtime_error("Could not open
    HKEY_LOCAL_MACHINE\\Software\\WOW6432Node for reading");
    }
    RegCloseKey(hKeySoftware);*/

    HKEY hKeyPassthrough;
    if ((res = RegOpenKeyEx(hKeySoftware, "PassThruSupport.04.04", 0, KEY_READ,
                            &hKeyPassthrough)) != ERROR_SUCCESS) {
        RegCloseKey(hKeySoftware);
        if (res == ERROR_FILE_NOT_FOUND) {
            // If this entry does not exist, then no PassThru interfaces are
            // installed
            return;
        }
        throw std::runtime_error(
            "Could not open "
            "HKEY_LOCAL_MACHINE\\Software\\PassThruSupport.04.04 for reading");
    }

    // We don't need to use this key anymore
    RegCloseKey(hKeySoftware);

    char keyValue[255];
    DWORD keySize;
    DWORD dwIndex = 0;
    do {
        keySize = sizeof(keyValue);
        res = RegEnumKeyEx(hKeyPassthrough, dwIndex++, keyValue, &keySize,
                           nullptr, nullptr, nullptr, nullptr);
        if (res == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (res != ERROR_SUCCESS) {
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error(
                "Error while enumerating passthrough devices");
        }

        HKEY hKeyDevice;
        if (RegOpenKeyEx(hKeyPassthrough, keyValue, 0, KEY_READ, &hKeyDevice) !=
            ERROR_SUCCESS) {
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error(
                "Error while opening passthrough device entry");
        }

        j2534::Info info;

        DWORD keyType;
        keySize = sizeof(keyValue);
        if (RegQueryValueEx(hKeyDevice, "Name", nullptr, &keyType,
                            reinterpret_cast<uint8_t *>(keyValue),
                            &keySize) != ERROR_SUCCESS) {
            RegCloseKey(hKeyDevice);
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error("Error querying passthrough device name");
        }

        info.name = keyValue;

        keySize = sizeof(keyValue);
        if (RegQueryValueEx(hKeyDevice, "FunctionLibrary", nullptr, &keyType,
                            reinterpret_cast<uint8_t *>(keyValue),
                            &keySize) != ERROR_SUCCESS) {
            RegCloseKey(hKeyDevice);
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error(
                "Error querying passthrough device function library");
        }

        info.functionLibrary = keyValue;

        // Check for CAN support
        DWORD can;
        keySize = sizeof(DWORD);
        if (RegQueryValueEx(hKeyDevice, "CAN", nullptr, &keyType,
                            reinterpret_cast<uint8_t *>(&can),
                            &keySize) == ERROR_SUCCESS) {
            if (can) {
                info.protocols = info.protocols | DataLinkProtocol::Can;
            }
        }

        Logger::info("Detected J2534 DLL: " + info.name);
        auto j2534 = j2534::J2534::create(std::move(info));
        /* Create the autodetect setting */
        auto settings = std::static_pointer_cast<J2534Settings>(
            InterfaceSettings::create(InterfaceType::J2534));
        settings->setInterface(j2534);
        settings->setName(j2534->name());
        InterfaceManager::get().addAuto(settings);

        interfaces_.push_back(std::move(j2534));

        RegCloseKey(hKeyDevice);
    } while (res == ERROR_SUCCESS);
    RegCloseKey(hKeyPassthrough);
}
