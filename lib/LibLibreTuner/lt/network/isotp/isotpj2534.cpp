#include "isotpj2534.h"
#include <iostream>

namespace lt::network
{
IsoTpJ2534::IsoTpJ2534(j2534::DevicePtr device, IsoTpOptions options)
    : device_(std::move(device)), channel_(device_->connect(j2534::Protocol::ISO15765, 0, options.baudrate)), options_(options)
{
    // Setup the filter
    j2534::PASSTHRU_MSG msgMask{};
    msgMask.ProtocolID = static_cast<uint32_t>(j2534::Protocol::ISO15765);
    msgMask.RxStatus = 0;
    msgMask.TxFlags = ISO15765_FRAME_PAD;
    msgMask.Timestamp = 0;
    msgMask.DataSize = 4;
    msgMask.ExtraDataIndex = 0;
    msgMask.Data[0] = 0xFF;
    msgMask.Data[1] = 0xFF;
    msgMask.Data[2] = 0xFF;
    msgMask.Data[3] = 0xFF;

    j2534::PASSTHRU_MSG msgPattern = msgMask;
    uint32_t id = options_.destId;
    msgPattern.Data[0] = (id & 0xFF000000U) >> 24U;
    msgPattern.Data[1] = (id & 0xFF0000U) >> 16U;
    msgPattern.Data[2] = (id & 0xFF00U) >> 8U;
    msgPattern.Data[3] = id & 0xFFU;

    j2534::PASSTHRU_MSG msgFlowControl = msgMask;
    uint32_t flow_id = options_.sourceId;
    msgFlowControl.Data[0] = (flow_id & 0xFF000000U) >> 24U;
    msgFlowControl.Data[1] = (flow_id & 0xFF0000U) >> 16U;
    msgFlowControl.Data[2] = (flow_id & 0xFF00U) >> 8U;
    msgFlowControl.Data[3] = flow_id & 0xFFU;

    uint32_t msgId;
    channel_.startMsgFilter(FLOW_CONTROL_FILTER, &msgMask, &msgPattern, &msgFlowControl, msgId);
}

void IsoTpJ2534::recv(IsoTpPacket & result)
{
    while (true)
    {
        j2534::PASSTHRU_MSG msg{};
        msg.ProtocolID = static_cast<uint32_t>(j2534::Protocol::ISO15765);

        uint32_t pNumMsgs = 1;
        channel_.readMsgs(&msg, pNumMsgs, options_.timeout.count());

        // Fill buffer
        if (msg.DataSize <= 4)
        {
            // The message does not fit the CAN ID
            continue;
        }
        uint32_t id = (msg.Data[0] << 24U) | (msg.Data[1] << 16U) | (msg.Data[2] << 8U) | (msg.Data[3]);
        if (id != options_.destId)
            continue;
        result.setData(std::next(std::begin(msg.Data), 4), msg.DataSize - 4);
        return;
    }
}

void IsoTpJ2534::request(const IsoTpPacket & req, IsoTpPacket & result)
{
    send(req);
    recv(result);
}

void IsoTpJ2534::send(const IsoTpPacket & packet)
{
    j2534::PASSTHRU_MSG msg{};
    msg.ProtocolID = static_cast<uint32_t>(j2534::Protocol::ISO15765);
    msg.TxFlags = ISO15765_FRAME_PAD;

    if (packet.size() > 4124)
        throw std::runtime_error("IsoTp packet exceeds maximum size (4124)");

    uint32_t id = options_.sourceId;
    msg.Data[0] = (id & 0xFF000000U) >> 24U;
    msg.Data[1] = (id & 0xFF0000U) >> 16U;
    msg.Data[2] = (id & 0xFF00U) >> 8U;
    msg.Data[3] = id & 0xFFU;

    std::copy(packet.begin(), packet.end(), std::next(std::begin(msg.Data), 4));
    msg.DataSize = packet.size() + 4;

    uint32_t numMsgs = 1;
    // TODO: Configurable timeout (100ms should be good for now, right?)
    channel_.writeMsgs(&msg, numMsgs, 100);
    if (numMsgs != 1)
        throw std::runtime_error("Message write timed out");
}

}