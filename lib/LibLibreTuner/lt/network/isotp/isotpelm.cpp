#include "isotpelm.h"

#include "../../support/util.hpp"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>

namespace lt::network {
namespace detail {
inline uint8_t hexToInt(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'z') {
        return 0xA + static_cast<uint8_t>(c - 'a');
    }
    if (c >= 'A' && c <= 'Z') {
        return 0xA + static_cast<uint8_t>(c - 'A');
    }
    throw std::runtime_error(std::string("invalid hex character: ") + c);
}

template <typename Iter> std::string decodeHex(Iter begin, Iter end) {
    std::string decoded;
    for (auto it = begin; it != end; ++it) {
        uint8_t byte = hexToInt(*it) << 4;
        if (++it == end) {
            break;
        }
        byte |= hexToInt(*it);
        decoded += static_cast<char>(byte);
    }
    return decoded;
}
} // namespace detail

IsoTpElm::IsoTpElm(Elm327Ptr device, IsoTpOptions options)
    : device_(std::move(device)), options_(options) {
    assert(device_);
    if (!device_->isOpen()) {
        throw std::runtime_error("Elm327 device is not open");
    }

    // Disable  echo
    device_->setEcho(false);
    // Set protocol
    device_->setProtocol(ElmProtocol::ISO_15765_4_CAN_11bit_500);
    // Disable printing spaces
    device_->setPrintSpaces(false);
    // Disable printing headers
    device_->setHeaders(false);

    updateOptions();
}

void IsoTpElm::recv(IsoTpPacket &result) {
    if (buffer_.empty()) {
        throw std::runtime_error("no ressponses remaining from last request");
    }
    result = std::move(buffer_.front());
    buffer_.pop();
}

void IsoTpElm::request(const IsoTpPacket &req, IsoTpPacket &result) {
    send(req);

    if (buffer_.empty()) {
        throw std::runtime_error("received no response");
    }

    result = std::move(buffer_.front());
    buffer_.pop();
}

void IsoTpElm::send(const IsoTpPacket &packet) {
    // Format packet into hex string
    std::stringstream ss;
    for (uint8_t ch : packet) {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<uint32_t>(ch);
    }

    std::vector<std::string> response = device_->sendCommand(ss.str());
    processResponse(response);
}

void IsoTpElm::setOptions(const IsoTpOptions &options) {
    options_ = options;
    updateOptions();
}

void IsoTpElm::updateOptions() {
    device_->setHeader(options_.sourceId);
    device_->setCanReceiveAddress11(options_.destId);
    // TODO: set timeout
}

void IsoTpElm::processResponse(std::vector<std::string> &response) {
    IsoTpPacket packet;

    int expectedLength = 0;

    for (std::string &line : response) {
        // Strip whitespace
        lt::remove_whitespace(line);
        if (line.empty()) {
            continue;
        }

        if (line.size() == 3) {
            // Multi-line packet length

            if (!packet.empty()) {
                // Last multi-line response was incomplete
                throw std::runtime_error(
                    "message did not meet expected length");
            }
            std::size_t pos;
            expectedLength = std::stoul(line, &pos, 16);
            if (pos != 3) {
                // Non-numeric character somewhere
                throw std::runtime_error("unexpected character in response: " +
                                         line);
            }
            continue;
        }

        auto delim = std::find(line.begin(), line.end(), ':');
        if (delim != line.end()) {
            // Part of multi-line packet
            auto msgBegin = delim + 1;
            std::string message = detail::decodeHex(msgBegin, line.end());
            expectedLength -= message.size();
            if (expectedLength < 0) {
                // Remove extra bytes
                std::size_t toRemove =
                    std::min<std::size_t>(message.size(), -expectedLength);
                message.erase(message.begin() + (message.size() - toRemove),
                              message.end());
                expectedLength += toRemove;
            }
            // Can ranges please come sooner?
            packet.append(reinterpret_cast<const uint8_t *>(message.c_str()),
                          message.length());

            if (expectedLength <= 0) {
                buffer_.emplace(std::move(packet));
                packet.clear();
            }
            continue;
        }

        if (!packet.empty()) {
            // The last message did not meet the expected length.
            throw std::runtime_error("message did not meet expected length");
        }

        // Single-line message
        std::string message = detail::decodeHex(line.begin(), line.end());
        packet.append(reinterpret_cast<const uint8_t *>(message.c_str()),
                      message.length());
        buffer_.emplace(std::move(packet));
        packet.clear();
    }
}
} // namespace lt::network