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

#include "j2534.h"
#include "logger.h"

#include <cassert>
#include <sstream>

#include <windows.h>
namespace j2534 {

void J2534::init() { load(); }

DevicePtr J2534::open(const char *port) {
    assert(initialized());

    Logger::debug("Opening J2534 device");

    uint32_t deviceId = 0;
    long res;
    if ((res = PassThruOpen(const_cast<void*>(reinterpret_cast<const void*>(port)), &deviceId)) != 0) {
        if (res == 0x8) { // ERR_DEVICE_NOT_CONNECTED
            // Return nullptr. Don't throw an exception,
            // because the absence of a device is not an exceptional error
            Logger::info("PassThru device not connected");
            return nullptr;
        }
        Logger::warning("PassThruOpen failed");
        throw Error(lastError());
    }
    Logger::debug("Opened j2534 device " + std::to_string(deviceId));
    return std::make_shared<Device>(shared_from_this(), deviceId);
}

void J2534::close(uint32_t device) {
    assert(initialized());

    long res;
    if ((res = PassThruClose(device)) != 0) {
        throw Error(lastError());
    }
}

uint32_t J2534::connect(uint32_t device, Protocol protocol, uint32_t flags,
                        uint32_t baudrate) {
    assert(initialized());

    long res;
    uint32_t channel;
    Logger::debug("PassThruConnect(" + std::to_string(device) + ", " +
                  std::to_string(static_cast<uint32_t>(protocol)) + ", " +
                  std::to_string(flags) + ", " + std::to_string(baudrate) +
                  ")");
    if ((res = PassThruConnect(device, static_cast<uint32_t>(protocol), flags,
                               baudrate, &channel)) != 0) {
        Logger::warning("PassThruConnect failed");
        throw Error(lastError());
    }
    Logger::debug("Connected j2534 channel " + std::to_string(channel));

    return channel;
}

void J2534::readMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                     uint32_t timeout) {
    assert(initialized());
    int32_t res = PassThruReadMsgs(channel, pMsg, &pNumMsgs, timeout);
    if (res != 0) {
        Logger::warning("PassThruReadMsgs failed");
        throw Error(lastError());
    }
}

void J2534::writeMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                      uint32_t timeout) {
    assert(initialized());
    int32_t res = PassThruWriteMsgs(channel, pMsg, &pNumMsgs, timeout);
    if (res != 0) {
        Logger::warning("PassThruWriteMsgs failed");
        throw Error(lastError());
    }
}

void J2534::startMsgFilter(uint32_t channel, uint32_t type,
                           const PASSTHRU_MSG *pMaskMsg,
                           const PASSTHRU_MSG *pPatternMsg,
                           const PASSTHRU_MSG *pFlowControlMsg,
                           uint32_t &pMsgID) {
    assert(initialized());
    int32_t res = PassThruStartMsgFilter(channel, type, pMaskMsg, pPatternMsg,
                                         pFlowControlMsg, &pMsgID);
    if (res != 0) {
        Logger::warning("PassThruStartMsgFilter failed");
        throw Error(lastError());
    }
}

void J2534::disconnect(uint32_t channel) {
    assert(initialized());

    long res;
    if ((res = PassThruDisconnect(channel)) != 0) {
        throw Error(lastError());
    }
}

std::string J2534::lastError() {
    char msg[80];
    PassThruGetLastError(msg);
    return std::string(msg);
}

J2534Ptr J2534::create(Info &&info) {
    return std::make_shared<J2534>(std::move(info));
}

J2534::~J2534() {
    if (hDll_) {
        CloseHandle(hDll_);
    }
}

void J2534::load() {
    Logger::debug("Loading " + info_.functionLibrary);
    if ((hDll_ = LoadLibrary(info_.functionLibrary.c_str())) == nullptr) {
        std::stringstream ss;
        ss << std::hex << GetLastError();
        throw Error("Failed to load library " + info_.functionLibrary + ": 0x" +
                    ss.str());
    }
    Logger::debug("Loaded library");
    Logger::debug("Loading library functions");
    PassThruOpen = reinterpret_cast<PassThruOpen_t>(getProc("PassThruOpen"));
    PassThruClose = reinterpret_cast<PassThruClose_t>(getProc("PassThruClose"));
    PassThruConnect =
        reinterpret_cast<PassThruConnect_t>(getProc("PassThruConnect"));
    PassThruDisconnect =
        reinterpret_cast<PassThruDisconnect_t>(getProc("PassThruDisconnect"));
    PassThruIoctl = reinterpret_cast<PassThruIoctl_t>(getProc("PassThruIoctl"));
    PassThruReadVersion =
        reinterpret_cast<PassThruReadVersion_t>(getProc("PassThruReadVersion"));
    PassThruGetLastError = reinterpret_cast<PassThruGetLastError_t>(
        getProc("PassThruGetLastError"));
    PassThruReadMsgs =
        reinterpret_cast<PassThruReadMsgs_t>(getProc("PassThruReadMsgs"));
    PassThruStartMsgFilter = reinterpret_cast<PassThruStartMsgFilter_t>(
        getProc("PassThruStartMsgFilter"));
    PassThruStopMsgFilter = reinterpret_cast<PassThruStopMsgFilter_t>(
        getProc("PassThruStopMsgFilter"));
    PassThruWriteMsgs =
        reinterpret_cast<PassThruWriteMsgs_t>(getProc("PassThruWriteMsgs"));
    PassThruStartPeriodicMsg = reinterpret_cast<PassThruStartPeriodicMsg_t>(
        getProc("PassThruStartPeriodicMsg"));
    PassThruStopPeriodicMsg = reinterpret_cast<PassThruStopPeriodicMsg_t>(
        getProc("PassThruStopPeriodicMsg"));
    PassThruSetProgrammingVoltage =
        reinterpret_cast<PassThruSetProgrammingVoltage_t>(
            getProc("PassThruSetProgrammingVoltage"));

    Logger::debug("Loaded library functions");

    // If all exports were found (no exceptions were thrown), we can set loaded_
    // to true
    loaded_ = true;
}

void *J2534::getProc(const char *proc) {
    assert(hDll_);
    void *func = reinterpret_cast<void *>(
        GetProcAddress(reinterpret_cast<HMODULE>(hDll_), proc));
    if (!func) {
        throw Error("Failed to get procedure from dll: " + std::string(proc));
    }
    return func;
}

Device::Device(const J2534Ptr &j2534, uint32_t device)
    : j2534_(j2534), device_(device) {}

Device::~Device() {
    Logger::debug("Destructing J2534 device");
    close();
}

void Device::close() {
    if (valid()) {
        j2534_->close(device_);
        j2534_.reset();
    }
}

Channel Device::connect(Protocol protocol, uint32_t flags, uint32_t baudrate) {
    assert(valid());
    Logger::debug("[j2534::Device] Connecting to J2534 channel");

    return Channel(j2534_, shared_from_this(),
                   j2534_->connect(device_, protocol, flags, baudrate));
}

Device::Device(Device &&dev) {
    device_ = dev.device_;
    j2534_ = std::move(dev.j2534_);
}

Channel::~Channel() {
    if (valid()) {
        j2534_->disconnect(channel_);
    }
}

Channel::Channel(Channel &&chann)
    : j2534_(std::move(chann.j2534_)), channel_(chann.channel_) {}

void Channel::readMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                       uint32_t timeout) {
    assert(valid());
    j2534_->readMsgs(channel_, pMsg, pNumMsgs, timeout);
}

void Channel::writeMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                        uint32_t timeout) {
    assert(valid());
    j2534_->writeMsgs(channel_, pMsg, pNumMsgs, timeout);
}

void Channel::startMsgFilter(uint32_t type, const PASSTHRU_MSG *pMaskMsg,
                             const PASSTHRU_MSG *pPatternMsg,
                             const PASSTHRU_MSG *pFlowControlMsg,
                             uint32_t &pMsgID) {
    assert(valid());
    j2534_->startMsgFilter(channel_, type, pMaskMsg, pPatternMsg,
                           pFlowControlMsg, pMsgID);
}






std::vector<Info> detect_interfaces() {
    std::vector<Info> interfaces;
    // Search HKEY_LOCAL_MACHINE\SOFTWARE\PassThruSupport.04.04 for connected
    // interfaces
    LSTATUS res;

    HKEY hKeyPassthrough;
    if ((res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\PassThruSupport.04.04", 0, KEY_ENUMERATE_SUB_KEYS,
                            &hKeyPassthrough)) != ERROR_SUCCESS) {
        //RegCloseKey(hKeySoftware);
        if (res == ERROR_FILE_NOT_FOUND) {
            // If this entry does not exist, then no PassThru interfaces are
            // installed
            return interfaces;
        }
        throw std::runtime_error(
                "Could not open "
                "HKEY_LOCAL_MACHINE\\Software\\PassThruSupport.04.04 for reading");
    }

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
                info.protocols = info.protocols | datalink::Protocol::Can;
            }
        }

        Logger::info("Detected J2534 DLL: " + info.name);

        interfaces.emplace_back(std::move(info));

        RegCloseKey(hKeyDevice);
    } while (res == ERROR_SUCCESS);
    RegCloseKey(hKeyPassthrough);
    return interfaces;
}

} // namespace j2534
