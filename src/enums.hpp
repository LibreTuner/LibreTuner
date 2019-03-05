#ifndef L_ENUMS_H
#define L_ENUMS_H

#include <cstdint>
#include <cassert>

enum class DownloadMode {
    None = 0,
    Mazda23,
};



enum class FlashMode {
    None = 0,
    T1, // Uses a CAN interface. Supported: Mazdaspeed 6
};



enum class TableType {
    Uint8,
    Uint16,
    Uint32,
    Float,
    Int8,
    Int16,
    Int32,
};



enum class DataLogMode {
    None,
    Uds,
};



inline std::size_t tableTypeSize(TableType type) {
    switch (type) {
        case TableType::Float:
            return sizeof(float);
        case TableType::Uint8:
            return sizeof(uint8_t);
        case TableType::Uint16:
            return sizeof(uint16_t);
        case TableType::Uint32:
            return sizeof(uint32_t);
        case TableType::Int8:
            return sizeof(int8_t);
        case TableType::Int16:
            return sizeof(int16_t);
        case TableType::Int32:
            return sizeof(int32_t);
    }
    
    assert(false && "unimplemented");
}

#endif // L_ENUMS_H
