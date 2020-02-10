#include "device.h"
#include <stdexcept>

#include <asm/termios.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <stropts.h>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace serial {

namespace detail {
/*
constexpr speed_t toTermBaud(Baudrate baudrate) {
    switch(baudrate) {
        case Baudrate::BR50: return B50;
        case Baudrate::BR75: return B75;
        case Baudrate::BR110: return B110;
        case Baudrate::BR134: return B134;
        case Baudrate::BR150: return B150;
        case Baudrate::BR200: return B200;
        case Baudrate::BR300: return B300;
        case Baudrate::BR600: return B600;
        case Baudrate::BR1200: return B1200;
        case Baudrate::BR1800: return B1800;
        case Baudrate::BR2400: return B2400;
        case Baudrate::BR4800: return B4800;
        case Baudrate::BR9600: return B9600;
        case Baudrate::BR19200: return B19200;
        case Baudrate::BR38400: return B38400;
        case Baudrate::BR57600: return B57600;
        case Baudrate::BR115200: return B115200;
        case Baudrate::BR230400: return B230400;
    }
    assert("Invalid baudrate");
    return 0;
}*/

constexpr int databits(DataBits bits) {
    switch (bits) {
    case DataBits::DB5:
        return CS5;
    case DataBits::DB6:
        return CS6;
    case DataBits::DB7:
        return CS7;
    case DataBits::DB8:
        return CS8;
    }
    assert("Invalid databits");
    return 0;
}

} // namespace detail

void Device::open() {
    if (fd_ != -1) {
        throw std::runtime_error("attempt to reopen serial port");
    }

    int flags = O_NOCTTY;
    switch (settings_.mode) {
    case Mode::Read:
        flags |= O_RDONLY;
        break;
    case Mode::Write:
        flags |= O_WRONLY;
        break;
    case Mode::ReadWrite:
        flags |= O_RDWR;
        break;
    }

    fd_ = ::open(port_.c_str(), flags);

    if (fd_ == -1) {
        throw std::runtime_error(std::string("error opening serial port: ") +
                                 strerror(errno));
    }

    updateSettings();
}

void Device::updateSettings() {
    if (fd_ == -1) {
        // No device to update
        return;
    }

    termios2 termSettings{};
    ioctl(fd_, TCGETS2, &termSettings);

    // Set parity
    switch (settings_.parity) {
    case Parity::None:
        termSettings.c_cflag &= ~PARENB;
        break;
    case Parity::Odd:
        termSettings.c_cflag |= PARENB | PARODD;
        break;
    case Parity::Even:
        termSettings.c_cflag |= PARENB;
        termSettings.c_cflag &= ~PARODD;
        break;
    }

    // Set stop bits
    switch (settings_.stopBits) {
    case StopBits::One:
        termSettings.c_cflag &= ~CSTOPB;
        break;
    case StopBits::Two:
        termSettings.c_cflag |= CSTOPB;
        break;
    }

    // Clear character size mask
    termSettings.c_cflag &= ~CSIZE;
    // Set data bits
    termSettings.c_cflag |= detail::databits(settings_.dataBits);

    termSettings.c_iflag &= ~IGNBRK;
    // termSettings.c_lflag = 0;
    // termSettings.c_oflag = 0;
    termSettings.c_oflag &= ~OPOST; /*No Output Processing*/

    // Set blocking
    termSettings.c_cc[VMIN] = 0;
    termSettings.c_cc[VTIME] = 5; // 0.5s timeout

    // Disable hardware flow control
    termSettings.c_cflag &= ~CRTSCTS;
    // Enable receiver, ignore modem control lines
    termSettings.c_cflag |= CREAD | CLOCAL;

    // Disable software flow control
    termSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Non canonical mode
    termSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // Set baudrate
    if (settings_.baudrate != 0) {
        termSettings.c_cflag &= ~CBAUD;
        termSettings.c_cflag |= BOTHER;
        termSettings.c_ispeed = settings_.baudrate;
        termSettings.c_ospeed = settings_.baudrate;
    }

    // Set attributes
    if ((ioctl(fd_, TCSETS2, &termSettings)) != 0) {
        throw std::runtime_error(
            std::string("error settings serial attributes: ") +
            strerror(errno));
    }
}

void Device::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

void Device::write(const std::string &data) {
    if (!isOpen()) {
        throw std::runtime_error("attempt to write to closed socket");
    }

    int amount = ::write(fd_, data.c_str(), data.size());
    if (amount == -1) {
        throw std::runtime_error(
            std::string("error while writing to serial: ") + strerror(errno));
    }
    if (amount != data.size()) {
        throw std::runtime_error("could not write full message to serial "
                                 "device. Expected to write " +
                                 std::to_string(data.size()) + ", wrote " +
                                 std::to_string(amount));
    }
}

Device::~Device() { close(); }

int Device::read(char *buffer, int amount) {
    if (!isOpen()) {
        throw std::runtime_error("attempt to read from closed socket");
    }

    int res = ::read(fd_, buffer, amount);
    if (res == -1) {
        throw std::runtime_error(
            std::string("error while reading from serial: ") + strerror(errno));
    }

    return res;
}

bool startsWith(const std::string &s, const std::string &start) {
    return s.size() >= start.size() && std::equal(s.begin(), s.begin() + start.size(), start.begin(), start.end());
}

std::vector<std::string> enumeratePorts() {
    std::vector<std::string> ports;

    for (auto &p : fs::directory_iterator("/sys/class/tty/")) {
        std::string filename = p.path().filename();
        if (startsWith(filename, "ttyS") || startsWith(filename, "ttyACM") || startsWith(filename, "ttyUSB")) {
            fs::path subsystemPath = fs::canonical(p.path() / "device" / "subsystem");
            if (subsystemPath.filename() != "platform") {
                fs::path devPath = fs::path("/dev/") / filename;
                if (fs::exists(devPath)) {
                    ports.emplace_back(devPath.string());
                }
            }
        }
    }

    return ports;
}

}