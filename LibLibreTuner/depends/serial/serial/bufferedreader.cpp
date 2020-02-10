#include "bufferedreader.h"

#include <stdexcept>
#include <array>
#include <algorithm>

namespace serial {


void BufferedReader::readSome() {
    int preSize = buffer_.size();
    buffer_.resize(buffer_.size() + 1024);

    int amountRead = device_.read(buffer_.data() + preSize, 1024);
    buffer_.resize(preSize + amountRead);

    if (amountRead == 0) {
        throw std::runtime_error("received 0 bytes from socket");
    }
}

std::string BufferedReader::read(int amount) {
    while (buffer_.size() < amount) {
        readSome();
    }
    std::string res(buffer_.begin(), buffer_.begin() + amount);
    buffer_.erase(buffer_.begin(), buffer_.begin() + amount);
    return res;
}

std::string BufferedReader::readLine(const std::string &stop) {
    constexpr std::array<char, 2> terminators{'\r', '\n'};

    skipWhitespace();

    auto pos = buffer_.end();
    std::size_t lastPos{0};
    while (true) {
        // Start searching from where we left off
        pos = std::find_first_of(buffer_.begin() + lastPos, buffer_.end(), terminators.begin(), terminators.end());
        // Check for stop
        if (!stop.empty() && buffer_.size() >= stop.size()) {
            if (std::equal(stop.begin(), stop.end(), buffer_.begin())) {
                // stop was found
                pos = buffer_.begin() + stop.size();
                break;
            }
        }

        if (pos != buffer_.end()) {
            break;
        }

        lastPos = buffer_.size();
        readSome();
        skipWhitespace();
    }

    std::string line(buffer_.begin(), pos);
    //std::advance(pos, 1);
    buffer_.erase(buffer_.begin(), pos);

    return line;
}

void BufferedReader::skipWhitespace() {
    buffer_.erase(buffer_.begin(), std::find_if_not(buffer_.begin(), buffer_.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    }));
}
}