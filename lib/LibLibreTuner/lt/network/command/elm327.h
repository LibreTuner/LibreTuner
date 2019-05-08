#ifndef LT_ELM327_H
#define LTELM327_H

#include <serial/device.h>
#include <serial/bufferedreader.h>
#include <memory>

namespace lt::network {

enum class ElmProtocol : uint8_t {
    Automatic = 0,
    SAE_J1850_PWM = 1,
    SAE_J1850_VPW = 2,
    ISO_9141_2 = 3,
    ISO_14230_4_KWP_5_Baud = 4, // 5 baud init
    ISO_14230_4_KWP_Fast = 5, // fast init
    ISO_15765_4_CAN_11bit_500 = 6, // ISO_TP w/ 11 bit id @ 500kbaud
    ISO_15765_4_CAN_29bit_500 = 7, // // ISO_TP w/ 29 bit id @ 500kbaud
    ISO_15765_4_CAN_11bit_250 = 8, // ISO_TP w/ 11 bit id @ 250kbaud
    ISO_15765_4_CAN_29bit_250 = 9, // // ISO_TP w/ 29 bit id @ 250kbaud
    SAE_J1939_CAN = 0xA, // 29 bit ID @ 250kbaud (adjustable)
    USER1_CAN = 0xB,
    USER2_CAN = 0xC,
};

class Elm327 {
public:
    Elm327();

    // Opens serial device
    void open();

    inline bool isOpen() const noexcept { return device_.isOpen(); }

    void setProtocol(ElmProtocol protocol);

    inline serial::Device &device() noexcept { return device_; }

    // If `echo` is true, enables echo. Else, disables echo.
    void setEcho(bool echo);

    // Enables or disables headers
    void setHeaders(bool headers);

    // Sets 11-bit CAN ID used for flow control
    void setCanFCId11(uint16_t id);

    // Set header (CAN ID)
    void setHeader(uint16_t header);

    // Set CAN receive ID (11 bit)
    void setCanReceiveAddress11(uint16_t address);

    // Appends CR to end of line and writes to serial
    void writeLine(std::string line);

    // Enable or disables printing spaces
    void setPrintSpaces(bool printSpaces);

    // Sets timeout byte (0 = 0ms, 0xFF = 1000ms)
    void setTimeout(uint8_t timeout);

    // Sends a command and waits for a response
    std::string sendCommand(const std::string &command);

    // Same as `sendCommand()` but throws an exception if the response is not "OK"
    void sendBasicCommand(const std::string &command);

private:
    serial::Device device_;
    serial::BufferedReader reader_;
};
using Elm327Ptr = std::shared_ptr<Elm327>;

}


#endif //LIBRETUNER_ELM327_H
