#ifndef SERIAL_DEVICE_H
#define SERIAL_DEVICE_H

#ifdef __linux__
#include "nix/device.h"
#elif WIN32
#include "windows/device.h"
#endif

#endif //SERIAL_DEVICE_H
