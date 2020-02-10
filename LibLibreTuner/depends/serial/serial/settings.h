#ifndef SERIAL_SETTINGS_H
#define SERIAL_SETTINGS_H

#include <cstdint>

namespace serial {

enum class Parity {
    None,
    Even,
    Odd,
};

/*
enum class Baudrate {
    BR50,
    BR75,
    BR110,
    BR134,
    BR150,
    BR200,
    BR300,
    BR600,
    BR1200,
    BR1800,
    BR2400,
    BR4800,
    BR9600,
    BR19200,
    BR38400,
    BR57600,
    BR115200,
    BR230400,
};*/

enum class StopBits {
    One,
    Two,
};

enum class Mode {
    Read,
    Write,
    ReadWrite,
};

enum class DataBits {
    DB5,
    DB6,
    DB7,
    DB8,
};

struct Settings {
    Parity parity{Parity::None};
    uint32_t baudrate{9600}; // If baudrate is 0 then the baudrate of the device will not be changed
    StopBits stopBits{StopBits::One};
    Mode mode{Mode::ReadWrite};
    DataBits dataBits{DataBits::DB8};
};

}


#endif //SERIAL_SETTINGS_H
