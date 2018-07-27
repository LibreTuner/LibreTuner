#include "j2534.h"

#include <cassert>

#include <Windows.h>
namespace j2534 {

void J2534::init()
{
    load();
}

Device J2534::open(char *port)
{
    assert(initialized());

    uint32_t deviceId;
    long res;
    if ((res = PassThruOpen(port, &deviceId)) != 0) {
        if (res == 0x8) { // ERR_DEVICE_NOT_CONNECTED
            // Return an invalid device. Don't throw an exception,
            // because the absence of a device is not an exceptional error
            return Device();
        }
        throw std::runtime_error(lastError());
    }
    return Device(shared_from_this(), deviceId);
}

void J2534::close(uint32_t device)
{
    assert(initialized());

    long res;
    if ((res = PassThruClose(device)) != 0) {
        throw std::runtime_error(lastError());
    }
}

Channel J2534::connect(uint32_t device, Protocol protocol, uint32_t flags, uint32_t baudrate)
{
    assert(initialized());

    long res;
    uint32_t channel;
    if ((res = PassThruConnect(device, static_cast<uint32_t>(protocol), flags, baudrate, &channel)) != 0) {
        throw std::runtime_error(lastError());
    }

    return Channel(shared_from_this(), channel);
}

void J2534::disconnect(uint32_t channel)
{
    assert(initialized());

    long res;
    if ((res = PassThruDisconnect(channel)) != 0) {
        throw std::runtime_error(lastError());
    }
}

std::string J2534::lastError()
{
    char msg[80];
    PassThruGetLastError(msg);
    return std::string(msg);
}

J2534Ptr J2534::create(Info &&info)
{
    return std::make_shared<J2534>(std::move(info));
}

J2534::~J2534()
{
    if (hDll_) {
        CloseHandle(hDll_);
    }
}

void J2534::load()
{
    if (!(hDll_ = LoadLibrary(info_.name.c_str()))) {
        throw std::runtime_error("Failed to load library " + info_.name);
    }

    PassThruOpen = static_cast<PassThruOpen_t>(getProc("PassThruOpen"));
    PassThruClose = static_cast<PassThruClose_t>(getProc("PassThruClose"));
    PassThruConnect = static_cast<PassThruConnect_t>(getProc("PassThruConnect"));
    PassThruDisconnect = static_cast<PassThruDisconnect_t>(getProc("PassThruDisconnect"));
    PassThruIoctl = static_cast<PassThruIoctl_t>(getProc("PassThruIoct"));
    PassThruReadVersion = static_cast<PassThruReadVersion_t>(getProc("PassThruReadVersion"));
    PassThruGetLastError = static_cast<PassThruGetLastError_t>(getProc("PassThruGetLastError"));
    PassThruReadMsgs = static_cast<PassThruReadMsgs_t>(getProc("PassThruReadMsgs"));
    PassThruStartMsgFilter = static_cast<PassThruStartMsgFilter_t>(getProc("PassThruStartMsgFilter"));
    PassThruStopMsgFilter = static_cast<PassThruStopMsgFilter_t>(getProc("PassThruStopMsgFilter"));
    PassThruWriteMsgs = static_cast<PassThruWriteMsgs_t>(getProc("PassThruWriteMsgs"));
    PassThruStartPeriodicMsg = static_cast<PassThruStartPeriodicMsg_t>(getProc("PassThruStartPeriodicMsg"));
    PassThruStopPeriodicMsg = static_cast<PassThruStopPeriodicMsg_t>(getProc("PassThruStopPeriodicMsg"));
    PassThruSetProgrammingVoltage = static_cast<PassThruSetProgrammingVoltage_t>(getProc("PassThruSetProgrammingVoltage"));

    // If all exports were found (no exceptions were thrown), we can set loaded_ to true
    loaded_ = true;
}

void *J2534::getProc(const char *proc)
{
    assert(hDll_);
    void *func = static_cast<void*>(GetProcAddress(hDll_, proc));
    if (!func) {
        throw std::runtime_error("Failed to get procedure from dll: " + std::string(proc));
    }
    return func;
}

Device::Device(const J2534Ptr &j2534, uint32_t device) : j2534_(j2534), device_(device)
{

}

Device::~Device()
{
    close();
}

void Device::close()
{
    if (valid()) {
        j2534_->close(device_);
        j2534_.reset();
    }
}

Channel Device::connect(Protocol protocol, uint32_t flags, uint32_t baudrate)
{
    assert(valid());

    return j2534_->connect(device_, protocol, flags, baudrate);
}

Device::Device(Device &&dev)
{
    device_ = dev.device_;
    j2534_ = std::move(dev.j2534_);
}

Channel::Channel(Channel &&chann) : j2534_(std::move(chann.j2534_)), channel_(chann.channel_)
{
}

}
