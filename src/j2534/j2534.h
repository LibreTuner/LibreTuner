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

#ifndef J2534_H
#define J2534_H

#include <memory>
#include <mutex>
#include <string>

#include "datalink.h"

/////////////////////////
// PassThruConnect flags
/////////////////////////

#define CAN_29BIT_ID 0x00000100
#define ISO9141_NO_CHECKSUM 0x00000200
#define CAN_ID_BOTH 0x00000800
#define ISO9141_K_LINE_ONLY 0x00001000
#define SNIFF_MODE                                                             \
    0x10000000 // OP2.0: listens to a bus (e.g. CAN) without acknowledging


//////////////////
// RxStatus flags
//////////////////

#define TX_MSG_TYPE 0x00000001
#define START_OF_MESSAGE 0x00000002
#define ISO15765_FIRST_FRAME 0x00000002
#define RX_BREAK 0x00000004
#define TX_DONE 0x00000008
#define ISO15765_PADDING_ERROR 0x00000010
#define ISO15765_EXT_ADDR 0x00000080
#define ISO15765_ADDR_TYPE 0x00000080
//#define CAN_29BIT_ID						0x00000100 // (already defined
// above)

//////////////////
// TxStatus flags
//////////////////

#define ISO15765_FRAME_PAD 0x00000040
//#define ISO15765_ADDR_TYPE				0x00000080 // (already defined
// above)
//#define CAN_29BIT_ID						0x00000100 // (already defined
// above)
#define WAIT_P3_MIN_ONLY 0x00000200
#define SW_CAN_HV_TX 0x00000400   // OP2.0: Not supported
#define SCI_MODE 0x00400000       // OP2.0: Not supported
#define SCI_TX_VOLTAGE 0x00800000 // OP2.0: Not supported


////////////////
// Filter types
////////////////

#define PASS_FILTER 0x00000001
#define BLOCK_FILTER 0x00000002
#define FLOW_CONTROL_FILTER 0x00000003



namespace j2534 {

class Error : public std::runtime_error {
public:
    Error(const std::string &message) : std::runtime_error(message) {}
};

struct Info {
    std::string name;
    // Supported protocols
    DataLinkProtocol protocols;
    // DLL path
    std::string functionLibrary;
};



// J2534 API

struct PASSTHRU_MSG {
    uint32_t ProtocolID; /* vehicle network protocol */
    uint32_t RxStatus;   /* receive message status */
    uint32_t TxFlags;    /* transmit message flags */
    uint32_t Timestamp;  /* receive message timestamp(in microseconds) */
    uint32_t DataSize;   /* byte size of message payload in the Data array */
    uint32_t ExtraDataIndex;  /* start of extra data(i.e. CRC, checksum, etc) in
                                 Data array */
    unsigned char Data[4128]; /* message payload or data */
};

using PassThruOpen_t = int32_t (*)(void *, uint32_t *);
using PassThruClose_t = int32_t (*)(uint32_t);
using PassThruConnect_t = int32_t (*)(uint32_t, uint32_t, uint32_t, uint32_t,
                                      uint32_t *);
using PassThruDisconnect_t = int32_t (*)(uint32_t);
using PassThruReadMsgs_t = int32_t (*)(uint32_t, PASSTHRU_MSG *, uint32_t *,
                                       uint32_t);
using PassThruWriteMsgs_t = int32_t (*)(uint32_t, PASSTHRU_MSG *, uint32_t *,
                                        uint32_t);
using PassThruStartPeriodicMsg_t = int32_t (*)(uint32_t, const PASSTHRU_MSG *,
                                               uint32_t *, uint32_t);
using PassThruStopPeriodicMsg_t = int32_t (*)(uint32_t, uint32_t);
using PassThruStartMsgFilter_t = int32_t (*)(uint32_t, uint32_t,
                                             const PASSTHRU_MSG *,
                                             const PASSTHRU_MSG *,
                                             const PASSTHRU_MSG *, uint32_t *);
using PassThruStopMsgFilter_t = int32_t (*)(uint32_t, uint32_t);
using PassThruSetProgrammingVoltage_t = int32_t (*)(uint32_t, uint32_t);
using PassThruReadVersion_t = int32_t (*)(char *, char *, char *);
using PassThruGetLastError_t = int32_t (*)(char *);
using PassThruIoctl_t = int32_t (*)(uint32_t, uint32_t, void *, void *);


class J2534;
using J2534Ptr = std::shared_ptr<J2534>;

enum class Protocol {
    None = 0,
    J1850VPW = 1,
    J1850PWM = 2,
    ISO9141 = 3,
    ISO14230 = 4,
    CAN = 5,
    ISO15765 = 6,
    SCI_A_Engine = 7,
    SCI_A_Trans = 8,
    SCI_B_Engine = 9,
    SCI_B_Trans = 10,
};

enum class Ioctl {
    GetConfig = 0x01,
    SetConfig = 0x02,
    ReadVbatt = 0x03,
    FiveBaudInit = 0x04,
    FastInit = 0x05,
    ClearTxBuffer = 0x07,
    ClearRxBuffer = 0x08,
    ClearPeriodicMsgs = 0x09,
    ClearMsgFilters = 0x0A,
    ClearFunctMsgLookupTable = 0x0B,
    AddToFunctMsgLookupTable = 0x0C,
    DeleteFromFunctMsgLookupTable = 0x0D,
    ReadProgVoltage = 0x0E,
    // J2534-2 SW_CAN
    SwCanHs = 0x8000,
    SwCanNs = 0x8001,
    SetPollResponse = 0x8002,
    BecomeMaster = 0x8003,
};

class Device;
using DevicePtr = std::shared_ptr<Device>;

class Channel {
public:
    // Creates an invalid device
    Channel() = default;

    ~Channel();

    Channel(const Channel &) = delete;
    Channel(Channel &&chann);

    /* Reads `pNumMsgs` messages or until the timeout expires. If timeout is 0,
     * reads the buffer and returns immediately. Sets pNumMsgs to the amount of
     messages actually read. Refer to the j2534 spec for more information. */
    void readMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs, uint32_t timeout);

    /* Writes the array of `pMsg` (size `pNumMsgs`) until timeout expires. Is
     timeout is 0,
     * queues as many transmit messages as possible and returns immediately.
     Sets pNumMsgs to the amount of messages sent. Refer to the j2534 spec for
     more information. */
    void writeMsgs(PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs, uint32_t timeout);

    void startMsgFilter(uint32_t type, const PASSTHRU_MSG *pMaskMsg,
                        const PASSTHRU_MSG *pPatternMsg,
                        const PASSTHRU_MSG *pFlowControlMsg, uint32_t &pMsgID);

    /* Disconnects the channel from the j2534 device. The object
     * is in an invalid state after calling this method */
    void disconnect();

    bool valid() const { return !!j2534_; }

    // This should only be constructed internally.
    // Use J2534Device::connect
    Channel(const J2534Ptr &j2534, const DevicePtr &device, uint32_t channel)
        : j2534_(j2534), device_(device), channel_(channel) {}

private:
    J2534Ptr j2534_;
    DevicePtr device_;
    uint32_t channel_;
};

class Device : public std::enable_shared_from_this<Device> {
public:
    // This object should never be contructed by the client. Use
    // J2534::open instaed
    Device(const J2534Ptr &j2534, uint32_t device);
    // Creates an invalid device
    Device() = default;

    ~Device();

    // closes the J2534 device and all channels
    void close();

    /* Establishes a logical communication channel with the vehicle
     * network (via the PassThru device) using the specified network
     * layer protocol and selected protocol options. */
    Channel connect(Protocol protocol, uint32_t flags = 0,
                    uint32_t baudrate = 500000);

    Device(const Device &) = delete;
    Device(Device &&dev);

    bool valid() const { return !!j2534_; }

    uint32_t handle() const { return device_; }

private:
    J2534Ptr j2534_;
    uint32_t device_;
};

// TODO: Synchronize this all into one thread! (IMPORTANT!!!)
class J2534 : public std::enable_shared_from_this<J2534> {
public:
    // Initializes the interface by loading the DLL. May throw an exception
    void init();

    // Returns true if the interface's library has been loaded
    bool initialized() const { return loaded_; }

    // Opens a J2534 device. If no device is connected, returns nullptr
    DevicePtr open(char *port = nullptr);

    // Closes a  J2534 device
    void close(uint32_t device);

    // see Device::connect
    uint32_t connect(uint32_t device, Protocol protocol, uint32_t flags,
                     uint32_t baudrate);

    void readMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                  uint32_t timeout);
    void writeMsgs(uint32_t channel, PASSTHRU_MSG *pMsg, uint32_t &pNumMsgs,
                   uint32_t timeout);
    void startMsgFilter(uint32_t channel, uint32_t type,
                        const PASSTHRU_MSG *pMaskMsg,
                        const PASSTHRU_MSG *pPatternMsg,
                        const PASSTHRU_MSG *pFlowControlMsg, uint32_t &pMsgID);

    // Disconnects a logical communication channel
    void disconnect(uint32_t channel);

    std::string lastError();

    std::string name() const { return info_.name; }

    // Returns the protocols supported by the J2534 interface
    DataLinkProtocol protocols() const { return info_.protocols; }

    // Creates a J2534 interface. Must be initialized with init() before use.
    static J2534Ptr create(Info &&info);

    J2534(Info &&info) : info_(std::move(info)) {}

    ~J2534();

private:
    Info info_;

    void *hDll_ = nullptr;

    bool loaded_ = false;

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

} // namespace j2534

#endif // J2534_H
