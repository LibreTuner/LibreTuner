#include "j2534manager.h"
#include <windows.h>

J2534Manager::J2534Manager()
{

}

J2534Manager &J2534Manager::get()
{
    static J2534Manager manager;
    return manager;
}

void J2534Manager::rediscover()
{
    // Ugly C code incoming

    // Search HKEY_LOCAL_MACHINE\SOFTWARE\PassThruSupport.04.04 for connected interfaces
    HKEY hKeySoftware;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKeySoftware) != ERROR_SUCCESS) {
        throw std::runtime_error("Could not open HKEY_LOCAL_MACHINE\\Software for reading");
    }

    HKEY hKeyPassthrough;
    if (RegOpenKeyEx(hKeySoftware, "PassThruSupport.04.04", 0, KEY_READ, &hKeyPassthrough) != ERROR_SUCCESS) {
        RegCloseKey(hKeySoftware);
        throw std::runtime_error("Could not open HKEY_LOCAL_MACHINE\\Software\\PassThruSupport.04.04 for reading");
    }

    char keyValue[255];
    DWORD keySize;
    DWORD dwIndex = 0;
    LSTATUS res;
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

    } while (res == ERROR_SUCCESS);
    RegCloseKey(hKeyPassthrough);
}
