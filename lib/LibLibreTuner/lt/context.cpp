#include "libretuner.h"

namespace lt {

static LogCallback _logCallback;

void setLogCallback(LogCallback &&cb) {
    _logCallback = std::move(cb);
}

void log(const std::string &message) {
    if (_logCallback) {
        _logCallback(message);
    }
}

}
