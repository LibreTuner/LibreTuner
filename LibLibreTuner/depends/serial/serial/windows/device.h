#ifndef SERIAL_WINDOWS_DEVICE_H
#define SERIAL_WINDOWS_DEVICE_H

#include <string>
#include <vector>
#include <utility>

#include "../settings.h"

namespace serial {
class Device {
public:
    explicit Device(std::string port = "", Settings settings = Settings{})
        : port_(std::move(port)), settings_{settings} {}

    // Tries to open socket and apply settings
    void open();

    // If a socket is open, closes it
    void close();

    // Sets the device name. Applies on next open()
    inline void setPort(const std::string &port) noexcept { port_ = port; }

    // Sets settings. Applies on next open() or with updateSettings().
    inline void setSettings(Settings settings) noexcept {
        settings_ = settings;
    }

    // Returns current settings
    inline Settings settings() const noexcept { return settings_; }

    // Returns true if the socket has not been closed. May return true if the
    // connection was closed remotely.
    inline bool isOpen() const noexcept { return handle_ != nullptr; }

    // If the socket is open, applies current settings to it.
    void updateSettings();

    // Writes data to device
    void write(const std::string &data);

    // Reads up to `amount` bytes. Returns amount of bytes read.
    int read(char *buffer, int amount);

    ~Device();

private:
    void* handle_{nullptr};
    std::string port_;
    Settings settings_;
};

std::vector<std::string> enumeratePorts();

} // namespace serial

#endif // SERIAL_WINDOWS_DEVICE_H
