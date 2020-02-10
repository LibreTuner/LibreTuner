#include "device.h"

#include <Windows.h>

#include <cassert>
#include <stdexcept>

namespace serial {

namespace detail {
constexpr int bytesize(DataBits bits) {
    switch (bits) {
    case DataBits::DB5:
        return 5;
    case DataBits::DB6:
        return 6;
    case DataBits::DB7:
        return 7;
    case DataBits::DB8:
        return 8;
    }
    assert("Invalid databits");
    return 0;
}
}

void Device::open() {
    std::string realPort = std::string(R"(\\.\)") + port_;

    handle_ = CreateFile(realPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                         nullptr, // No Security
                         OPEN_EXISTING,
                         0, // Non Overlapped I/O
                         nullptr);

    if (handle_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("failed to open COM port");
    }

    updateSettings();

    // Set timeouts
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 500;

    if (SetCommTimeouts(handle_, &timeouts) == FALSE) {
        throw std::runtime_error("failed to set comm timeouts");
    }
}

void Device::close() {
    if (isOpen()) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
}

Device::~Device() {
    close();
}

void Device::write(const std::string &data) {
    if (!isOpen()) {
        throw std::runtime_error("attempt to write to closed socket");
    }
    DWORD written = 0;

    if (WriteFile(handle_, data.c_str(), data.size(), &written, nullptr) == FALSE) {
        throw std::runtime_error("error write writing to com device");
    }
    if (written != data.size()) {
        throw std::runtime_error("could not write full message to serial "
                                 "device. Expected to write " +
                                 std::to_string(data.size()) + ", wrote " +
                                 std::to_string(written));
    }
}

int Device::read(char *buffer, int amount) {
    DWORD amountRead = 0;
    if (ReadFile(handle_, buffer, amount, &amountRead, nullptr) == FALSE) {
        throw std::runtime_error("error while reading from comm device");
    }
    return amountRead;
}

void Device::updateSettings() {
    if (!isOpen()) {
        return;
    }
    // Control settings
    DCB controlSetting{};
    controlSetting.DCBlength = sizeof(DCB);

    if (GetCommState(handle_, &controlSetting) == FALSE) {
        throw std::runtime_error("failed to get comm state for port");
    }

    if (settings_.baudrate != 0) {
        controlSetting.BaudRate = settings_.baudrate;
    }
    controlSetting.ByteSize = detail::bytesize(settings_.dataBits);

    switch (settings_.stopBits) {
        case StopBits::Two:
            controlSetting.StopBits = TWOSTOPBITS;
            break;
        case StopBits::One:
            controlSetting.StopBits = ONESTOPBIT;
            break;
    }

    switch (settings_.parity) {
        case Parity::Odd:
            controlSetting.Parity = ODDPARITY;
            break;
        case Parity::Even:
            controlSetting.Parity = EVENPARITY;
            break;
        case Parity::None:
            controlSetting.Parity = NOPARITY;
            break;
    }

    if (SetCommState(handle_, &controlSetting) == FALSE) {
        throw std::runtime_error("failed to set comm state");
    }
}

std::vector<std::string> enumeratePorts() {
    std::vector<std::string> ports;

    LSTATUS res;

    HKEY hKeySerial;

    if ((res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hKeySerial)) != ERROR_SUCCESS) {
        if (res == ERROR_FILE_NOT_FOUND) {
            // File not found is not an error; no entries.
            return ports;
        }
        throw std::runtime_error(R"(failed to open HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM for reading)");
    }

    char keyValue[255];
    DWORD keySize;
    DWORD dwIndex = 0;
    do {
        keySize = sizeof(keyValue);
        res = RegEnumValue(hKeySerial, dwIndex++, keyValue, &keySize,
                           nullptr, nullptr, nullptr, nullptr);
        if (res == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (res != ERROR_SUCCESS) {
            RegCloseKey(hKeySerial);
            throw std::runtime_error(
                    "Error while enumerating serial devices");
        }

        std::string name(keyValue, keySize);

        keySize = sizeof(keyValue);
        if ((res = RegQueryValueExA(hKeySerial, name.c_str(), nullptr, nullptr, reinterpret_cast<BYTE*>(keyValue), &keySize)) !=
            ERROR_SUCCESS) {
            RegCloseKey(hKeySerial);
            throw std::runtime_error(
                    "Error while opening serial device entry");
        }

        ports.emplace_back(std::string(keyValue, keySize));
    } while (res == ERROR_SUCCESS);

    RegCloseKey(hKeySerial);

    return ports;
}

}
