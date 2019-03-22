#include "isotp.h"

#include <string>
#include <thread>

namespace lt {
namespace network {

namespace detail {

uint8_t calculate_st(std::chrono::microseconds time) {
    assert(time.count() >= 0);
    if (time.count() == 0) {
        return 0;
    }

    if (time >= std::chrono::milliseconds(1)) {
        return static_cast<uint8_t>(std::min<std::chrono::milliseconds::rep>(
            std::chrono::duration_cast<std::chrono::milliseconds>(time).count(),
            127));
    }
    uint8_t count = static_cast<uint8_t>(
        std::max<std::chrono::milliseconds::rep>(time.count() / 100, 1));
    return count + 0xF0;
}

std::chrono::microseconds calculate_time(uint8_t st) {
    if (st <= 127) {
        return std::chrono::milliseconds(st);
    }
    if (st >= 0xF1 && st <= 0xF9) {
        return std::chrono::microseconds((st - 0xF0) * 100);
    }
    return std::chrono::microseconds(0);
}

} // namespace detail

struct FlowControlFrame {
    uint8_t fcFlag, blockSize, st;
};

constexpr uint8_t typeSingle = 0;
constexpr uint8_t typeFirst = 1;
constexpr uint8_t typeConsec = 2;
constexpr uint8_t typeFlow = 3;

class MultiFrameReceiver {
public:
	MultiFrameReceiver(uint16_t size, IsoTpPacket& packet, Can& can, IsoTpOptions& options, IsoTp& protocol)
		: packet_(packet), can_(can), options_(options), protocol_(protocol), size_(size) {}

	void recv();

	uint8_t nextConsec();

private:
	void sendFlowControl();
	void recvConsecutiveFrames();

	IsoTpPacket &packet_;
	Can& can_;
	IsoTpOptions options_;
	IsoTp& protocol_;

	uint8_t consecIndex_{ 1 };
	uint16_t size_;
};

class MultiFrameSender {
public:
    MultiFrameSender(const IsoTpPacket &packet, Can &can, IsoTpOptions &options, IsoTp &protocol)
        : reader_(packet), can_(can), options_(options), protocol_(protocol) {}

    void send();

  private:
    FlowControlFrame recvFlowControl();
    void waitForFlowControl();
	// Sends consecutive frames until blocksize reaches 0
	// or the end of the packet is reached
    void sendConsecFrames();

    uint8_t nextConsec();

    IsoTpPacketReader reader_;
    Can &can_;
    IsoTpOptions options_;
	IsoTp &protocol_;

	std::chrono::microseconds separationTime_{0};
	uint8_t blockSize_{0};
    uint8_t consecIndex_{1};
};

IsoTpPacket::IsoTpPacket() {}

IsoTpPacket::IsoTpPacket(const uint8_t *data, size_t size)
    : data_(data, data + size) {}

void IsoTpPacket::setData(const uint8_t *data, size_t size) {
    data_.assign(data, data + size);
}

void IsoTpPacket::moveInto(std::vector<uint8_t> &data) {
    data = std::move(data_);
}

void IsoTpPacket::append(const uint8_t *data, size_t size) {
    data_.insert(data_.begin() + data_.size(), data, data + size);
}

IsoTp::IsoTp(CanPtr &&can, IsoTpOptions options)
    : can_(std::move(can)), options_(std::move(options)) {}

IsoTp::~IsoTp() = default;

void IsoTp::recv(IsoTpPacket& result)
{
	assert(can_);
	CanMessage message = recvNextFrame();
	uint8_t type = message[0] >> 4;
	if (type == typeSingle) {
		result.setData(message.message() + 1, message.length() - 1);
		return;
	}
	if (type == typeFirst) {
		uint16_t length = ((message[0] & 0x0F) << 8) | message[1];
        result.append(message.message() + 2, 6);
		MultiFrameReceiver receiver(length - 6, result, *can_, options_, *this);
		receiver.recv();
		return;
	}
	throw std::runtime_error("received invalid frame type. Expected " + std::to_string(typeSingle) + " or " + std::to_string(typeFirst) + ", got " + std::to_string(type));
}

void IsoTp::request(const IsoTpPacket& req, IsoTpPacket& result)
{
	send(req);
	recv(result);
}

void IsoTp::send(const IsoTpPacket &packet) {
	assert(can_);
    // Determine if packet will fit into a single frame
    if (packet.size() <= 7) {
        // Single frame
        sendSingleFrame(packet.data(), packet.size());
    } else {
		MultiFrameSender sender(packet, *can_, options_, *this);
		sender.send();
    }
}

void IsoTp::sendSingleFrame(const uint8_t *data, std::size_t size) {
    assert(can_);
    assert(size <= 7);

    CanMessage message;
    message.setId(options_.sourceId);
    message[0] = (typeSingle << 4) | (static_cast<uint8_t>(size));
    std::copy(data, data + size, message.message() + 1);
	message.setLength(size + 1);
    
    message.pad();

    can_->send(message);
}

std::vector<uint8_t> IsoTpPacketReader::next(std::size_t max) {
    std::size_t toRead = std::min(max, remaining());

    std::vector<uint8_t> rem(packet_.cbegin() + pointer_,
                             packet_.cbegin() + pointer_ + toRead);
    pointer_ += toRead;
    return rem;
}

std::size_t IsoTpPacketReader::next(uint8_t *dest, std::size_t max) {
    std::size_t toRead = std::min(max, remaining());
    std::copy(packet_.cbegin() + pointer_, packet_.cbegin() + pointer_ + toRead,
              dest);
    pointer_ += toRead;
	return toRead;
}

std::vector<uint8_t> IsoTpPacketReader::readRemaining() {
    std::vector<uint8_t> rem(packet_.cbegin() + pointer_, packet_.cend());
    pointer_ = packet_.size();
    return rem;
}

void MultiFrameSender::send() {
    // Send first frame
    CanMessage message;
    message.setId(options_.sourceId);
    message[0] = (typeFirst << 4) | ((reader_.remaining() & 0xF00) >> 8);
    message[1] = reader_.remaining() & 0xFF;

    std::size_t amountRead = reader_.next(message.message() + 2, 6);
    message.setLength(amountRead + 2);

    message.pad();
    
    can_.send(message);

    waitForFlowControl();
}

FlowControlFrame MultiFrameSender::recvFlowControl() {
    CanMessage message = protocol_.recvNextFrame(typeFlow);

    if (message.length() < 3) {
        throw std::runtime_error(
            "received invalid flow control response: too short");
    }

    FlowControlFrame frame;
    frame.fcFlag = message[0] & 0x0F;
    frame.blockSize = message[1];
    frame.st = message[2];
    return frame;
}

void MultiFrameSender::waitForFlowControl() {
    while (reader_.remaining() != 0) {
        FlowControlFrame frame;
        do {
            frame = recvFlowControl();
            if (frame.fcFlag == 2) {
                throw std::runtime_error("remote requested to abort transfer");
            }
        } while (frame.fcFlag == 1);

        separationTime_ = detail::calculate_time(frame.st);
        blockSize_ = frame.blockSize;

		sendConsecFrames();
    }
}

void MultiFrameSender::sendConsecFrames() {
	do {
		CanMessage message;
		message.setId(options_.sourceId);
		message[0] = (typeConsec << 4) | nextConsec();
		message.setLength(reader_.next(message.message() + 1, 7) + 1);
        message.pad();
		can_.send(message);

		std::this_thread::sleep_for(separationTime_);
	} while (reader_.remaining() != 0 && (blockSize_ == 0 || --blockSize_ != 0));
}

CanMessage IsoTp::recvNextFrame() {
    auto start = std::chrono::steady_clock::now();
    CanMessage message;
    while (can_->recv(message, options_.timeout) && (std::chrono::steady_clock::now() - start) < options_.timeout) {
        if (message.id() == options_.destId) {
            if (message.length() == 0) {
                throw std::runtime_error("received empty frame");
            }
            return message;
        }
    }
    throw std::runtime_error("timed out");
}

CanMessage IsoTp::recvNextFrame(uint8_t expectedType) {
    CanMessage message = recvNextFrame();
    uint8_t id = message[0] >> 4;
    if (id != expectedType) {
        throw std::runtime_error("received unexpected frame type. Got " +
                                 std::to_string(id) + ", expected " +
                                 std::to_string(expectedType));
    }
	return message;
}

uint8_t MultiFrameSender::nextConsec() {
    uint8_t index = consecIndex_++;
    if (consecIndex_ == 16) {
        consecIndex_ = 0;
    }
    return index;
}

void MultiFrameReceiver::recv()
{
	sendFlowControl();
	recvConsecutiveFrames();
}

uint8_t MultiFrameReceiver::nextConsec()
{
	uint8_t index = consecIndex_++;
	if (consecIndex_ == 16) {
		consecIndex_ = 0;
	}
	return index;
}

void MultiFrameReceiver::sendFlowControl()
{
	CanMessage message;
	message.setId(options_.sourceId);
	message.setLength(3);
	message[0] = (typeFlow << 4) | 0;
	message[1] = 0;
	message[2] = 0;
    message.pad();
	can_.send(message);
}

void MultiFrameReceiver::recvConsecutiveFrames()
{
	while (size_ != 0) {
		CanMessage frame = protocol_.recvNextFrame(typeConsec);
		uint8_t index = frame[0] & 0x0F;
		if (index != nextConsec()) {
			throw std::runtime_error("received invalid consecutive frame index");
		}

		uint16_t received = std::min<uint16_t>(frame.length() - 1, size_);

		packet_.append(frame.message() + 1, received);
		size_ -= received;
	}
}

} // namespace network
} // namespace lt
