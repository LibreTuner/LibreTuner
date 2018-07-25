#ifndef J2534_H
#define J2534_H

#include <memory>
#include <string>

#include "datalink.h"

#include <Windows.h>

struct J2534Info {
    std::string name;
    // Supported protocols
    DataLinkProtocol protocols;
    // DLL path
    std::string functionLibrary;
};



// J2534 API

struct PASSTHRU_MSG {
    unsigned long ProtocolID; /* vehicle network protocol */
    unsigned long RxStatus; /* receive message status */
    unsigned long TxFlags; /* transmit message flags */
    unsigned long Timestamp; /* receive message timestamp(in microseconds) */
    unsigned long DataSize; /* byte size of message payload in the Data array */
    unsigned long ExtraDataIndex; /* start of extra data(i.e. CRC, checksum, etc) in Data array */
    unsigned char Data[4128]; /* message payload or data */
};

using PassThruOpen_t = long (*) (void*, unsigned long*);
using PassThruClose_t = long (*) (unsigned long);
using PassThruConnect_t = long (*) (unsigned long, unsigned long, unsigned long, unsigned long, unsigned long*);
using PassThruDisconnect_t = long (*) (unsigned long);
using PassThruReadMsgs_t = long (*) (unsigned long, PASSTHRU_MSG*, unsigned long*, unsigned long);
using PassThruWriteMsgs_t = long (*) (unsigned long, PASSTHRU_MSG*, unsigned long*, unsigned long);
using PassThruStartPeriodicMsg_t = long (*) (unsigned long, PASSTHRU_MSG*, unsigned long*, unsigned long);
using PassThruStopPeriodicMsg_t = long (*) (unsigned long, unsigned long);
using PassThruStartMsgFilter_t = long (*) (unsigned long, unsigned long, PASSTHRU_MSG*, PASSTHRU_MSG*, PASSTHRU_MSG*, unsigned long*);
using PassThruStopMsgFilter_t = long (*) (unsigned long, unsigned long);
using PassThruSetProgrammingVoltage_t = long (*) (unsigned long, unsigned long);
using PassThruReadVersion_t = long (*) (char*, char*, char*);
using PassThruGetLastError_t = long (*) (char*);
using PassThruIoctl_t = long (*) (unsigned long, unsigned long, void*, void*);


class J2534;
using J2534Ptr = std::shared_ptr<J2534>;

class J2534Channel {
public:
    // This should only be constructed internally.
    // Use J2534Device::connect
    J2534Channel(const J2534Ptr &j2534, unsigned int channel) : j2534_(j2534), channel_(channel) {}
    // Creates an invalid device
    J2534Channel() = default;

    J2534Channel(const J2534Channel&) = delete;
    J2534Channel(J2534Channel&& chann);

    bool valid() const { return !!j2534_; }

private:
    J2534Ptr j2534_;
    unsigned int channel_;
};

class J2534Device {
public:
    // This object should never be contructed by the client. Use
    // J2534::open instaed
    J2534Device(const J2534Ptr &j2534, unsigned int device);
    // Creates an invalid device
    J2534Device() = default;

    J2534Device(const J2534Device&) = delete;
    J2534Device(J2534Device&& dev);

    bool valid() const { return !!j2534_; }

private:
    J2534Ptr j2534_;
    unsigned int device_;
};

class J2534 : public std::enable_shared_from_this<J2534>
{
public:
    // Initializes the interface by loading the DLL. May throw an exception
    void init();

    // Returns true if the interface's library has been loaded
    bool initialized() const;

    // Opens a J2534 device. If no device is connected, returns an invalid device
    // check J2534Device::valid()
    J2534Device open(char *port = nullptr);

    void close(unsigned int device);

    std::string lastError();

    std::string name() const { return info_.name; }

    // Returns the protocols supported by the J2534 interface
    DataLinkProtocol protocols() const { return info_.protocols; }

    // Creates a J2534 interface. Must be initialized with init() before use.
    static J2534Ptr create(J2534Info &&info);

    J2534(J2534Info &&info) : info_(std::move(info)) {}

    ~J2534();

private:
    J2534Info info_;

    HINSTANCE hDll_ = nullptr;

    // Loads the dll
    void load();

    void *getProc(const char *proc);

    PassThruOpen_t PassThruOpen{};
    PassThruClose_t PassThruClose{};
    PassThruConnect_t PassThruConnect{};
    PassThruDisconnect_t PassThruDisconnect{};
    PassThruIoctl_t PassThruIoctl{};
    PassThruReadVersion_t PassThruReadVersion{};
    PassThruGetLastError_t PassThruGetLastError{};
    PassThruReadMsgs_t PassThruReadMsgs{};
    PassThruStartMsgFilter_t PassThruStartMsgFilter{};
    PassThruStopMsgFilter_t PassThruStopMsgFilter{};
    PassThruWriteMsgs_t PassThruWriteMsgs{};
    PassThruStartPeriodicMsg_t PassThruStartPeriodicMsg{};
    PassThruStopPeriodicMsg_t PassThruStopPeriodicMsg{};
    PassThruSetProgrammingVoltage_t PassThruSetProgrammingVoltage{};
};

#endif // J2534_H
