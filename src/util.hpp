#ifndef UTIL_H
#define UTIL_H

/* Decodes an int32 encoded in big endian */
inline uint32_t toBEInt32(const uint8_t *data)
{
    return ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
}



inline void writeBEInt32(uint32_t i, uint8_t *data)
{
    data[0] = (i & 0xFF000000) >> 24;
    data[1] = (i & 0xFF0000) >> 16;
    data[2] = (i & 0xFF00) >> 8;
    data[3] = i & 0xFF;
}


/* Decodes an int32 encoded in little endian */
inline uint32_t toLEInt32(const uint8_t *data)
{
    return (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}



/* Decodes a float encoded in big endian */
inline float toBEFloat(const uint8_t *data)
{
    uint32_t raw = toBEInt32(data);
    return *(float*)(&raw);
}



/* Decodes a float encoded in little endian */
inline float toLEFloat(const uint8_t *data)
{
    uint32_t raw = toLEInt32(data);
    return *(float*)(&raw);
}





#endif
