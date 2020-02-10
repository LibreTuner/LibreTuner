#ifndef SERIAL_BUFFEREDREADER_H
#define SERIAL_BUFFEREDREADER_H

#include "device.h"
#include <vector>

namespace serial {
class BufferedReader {
public:
    explicit BufferedReader(Device &device) : device_(device) {}

    // Reads exactly `amount` bytes
    std::string read(int amount);

    // Reads a single line terminating in CR or LF
    // or if `stop` is read at the beginning of the line
    std::string readLine(const std::string &stop = "");

    // Clears buffer
    inline void clear() noexcept { buffer_.clear(); }

    // Clears whitespace at the beginning of the buffer
    void skipWhitespace();

private:
    std::vector<char> buffer_;
    Device &device_;

    void readSome();
};
}

#endif //LIBRETUNER_BUFFEREDREADER_H
