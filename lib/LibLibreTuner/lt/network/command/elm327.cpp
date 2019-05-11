#include "elm327.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace lt::network {

Elm327::Elm327(std::string port, serial::Settings serialSettings) : device_(port, serialSettings), reader_(device_) {}

void Elm327::open() { device_.open(); }

void Elm327::setProtocol(ElmProtocol protocol) {
    std::stringstream ss;
    ss << "AT SP " << std::hex << static_cast<uint32_t>(protocol);
    sendCommand(ss.str());
}

void Elm327::writeLine(std::string line) {
    if (!isOpen()) {
        throw std::runtime_error(
            "attempted to write line to closed connection");
    }
    line += "\r";

    device_.write(line);
}

std::vector<std::string> Elm327::sendCommand(const std::string &command) {
    writeLine(command);

    std::vector<std::string> response;

    std::string line;
    while (true) {
        line = reader_.readLine(">");
        if (line.empty()) {
            continue;
        }
        if (line == ">") {
            break;
        }

        if (line == "?") {
            throw std::runtime_error("received ? from elm");
        }

        if (line == "CAN ERROR") {
            throw std::runtime_error("received CAN ERROR");
        }
        if (line == "NO DATA") {
            throw std::runtime_error("received no data");
        }

        response.emplace_back(std::move(line));
    }
    return response;
}

void Elm327::sendBasicCommand(const std::string &command) {
    std::vector<std::string> response = sendCommand(command);
    if (response.size() != 1 || response.front() != "OK") {
        throw std::runtime_error("received invalid response, expected \"OK\"");
    }
}

void Elm327::setEcho(bool echo) {
    std::string command = "AT E ";
    command += echo ? '1' : '0';
    sendBasicCommand(command);
}

void Elm327::setHeaders(bool headers) {
    std::string command = "AT H ";
    command += headers ? '1' : '0';
    sendBasicCommand(command);
}

void Elm327::setCanFCId11(uint16_t id) {
    std::stringstream ss;
    ss << "AT FC SH " << std::setfill('0') << std::setw(3) << std::hex << id;
    sendBasicCommand(ss.str());
}

void Elm327::setHeader(uint16_t header) {
    std::stringstream ss;
    ss << "AT SH " << std::setfill('0') << std::setw(3) << std::hex << header;
    sendBasicCommand(ss.str());
}

void Elm327::setCanReceiveAddress11(uint16_t address) {
    std::stringstream ss;
    ss << "AT CRA " << std::setfill('0') << std::setw(3) << std::hex << address;
    sendBasicCommand(ss.str());
}

void Elm327::setPrintSpaces(bool printSpaces) {
    std::string command = "AT S ";
    command += printSpaces ? '1' : '0';
    sendBasicCommand(command);
}

void Elm327::setTimeout(uint8_t timeout) {
    std::stringstream ss;
    ss << "AT ST " << std::setfill('0') << std::setw(2) << std::hex << timeout;
    sendBasicCommand(ss.str());
}

} // namespace lt::network