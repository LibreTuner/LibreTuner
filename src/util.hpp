#ifndef UTIL_H
#define UTIL_H


/* Decodes a float encoded in big endian */
float toBEFloat(const uint8_t *data)
{
    uint32_t raw = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
    return *(float*)(&raw);
}


/* Decodes a float encoded in little endian */
float toLEFloat(const uint8_t *data)
{
    uint32_t raw = (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
    return *(float*)(&raw);
}





#endif
