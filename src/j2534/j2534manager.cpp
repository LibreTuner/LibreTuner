#include "j2534manager.h"
#include "j2534.h"

#include <Windows.h>

J2534Manager::J2534Manager()
{
}

J2534Manager &J2534Manager::get()
{
    static J2534Manager manager;
    return manager;
}

void J2534Manager::init()
{
    rediscover();
}

void J2534Manager::rediscover()
{
    interfaces_.clear();
    // Ugly C code incoming

    // Search HKEY_LOCAL_MACHINE\SOFTWARE\PassThruSupport.04.04 for connected interfaces
    HKEY hKeySoftware;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKeySoftware) != ERROR_SUCCESS) {
        throw std::runtime_error("Could not open HKEY_LOCAL_MACHINE\\Software for reading");
    }

    HKEY hKeyPassthrough;
    LSTATUS res;
    if ((res = RegOpenKeyEx(hKeySoftware, "PassThruSupport.04.04", 0, KEY_READ, &hKeyPassthrough)) != ERROR_SUCCESS) {
        RegCloseKey(hKeySoftware);
        if (res == ERROR_FILE_NOT_FOUND) {
            // If this entry does not exist, then no PassThru interfaces are installed
            return;
        }
        throw std::runtime_error("Could not open HKEY_LOCAL_MACHINE\\Software\\PassThruSupport.04.04 for reading");
    }

    // We don't need to use this key anymore
    RegCloseKey(hKeySoftware);

    char keyValue[255];
    DWORD keySize;
    DWORD dwIndex = 0;
    do {
        keySize = sizeof(keyValue);
        res = RegEnumKeyEx(hKeyPassthrough, dwIndex++, keyValue, &keySize, nullptr, nullptr, nullptr, nullptr);
        if (res == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (res != ERROR_SUCCESS) {
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error("Error while enumerating passthrough devices");
        }

        HKEY hKeyDevice;
        if (RegOpenKeyEx(hKeyPassthrough, keyValue, 0, KEY_READ, &hKeyDevice) != ERROR_SUCCESS) {
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error("Error while opening passthrough device entry");
        }

        j2534::Info info;

        DWORD keyType;
        keySize = sizeof(keyValue);
        if (RegQueryValueEx(hKeyDevice, "Name", nullptr, &keyType, reinterpret_cast<uint8_t*>(keyValue), &keySize) != ERROR_SUCCESS) {
            RegCloseKey(hKeyDevice);
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error("Error querying passthrough device name");
        }

        info.name = keyValue;

        keySize = sizeof(keyValue);
        if (RegQueryValueEx(hKeyDevice, "FunctionLibrary", nullptr, &keyType, reinterpret_cast<uint8_t*>(keyValue), &keySize) != ERROR_SUCCESS) {
            RegCloseKey(hKeyDevice);
            RegCloseKey(hKeyPassthrough);
            throw std::runtime_error("Error querying passthrough device function library");
        }

        info.functionLibrary = keyValue;

        // Check for CAN support
        DWORD can;
        keySize = sizeof(DWORD);
        if (RegQueryValueEx(hKeyDevice, "CAN", nullptr, &keyType, reinterpret_cast<uint8_t*>(&can), &keySize) == ERROR_SUCCESS) {
            if (can) {
                info.protocols = info.protocols | DataLinkProtocol::Can;
            }
        }

        interfaces_.push_back(j2534::J2534::create(std::move(info)));

        RegCloseKey(hKeyDevice);
    } while (res == ERROR_SUCCESS);
    RegCloseKey(hKeyPassthrough);
}
