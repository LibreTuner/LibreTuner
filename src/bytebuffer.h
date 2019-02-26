#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include <string>
#include <vector>

#include "util.hpp"

/**
 * @todo write docs
 */
class ByteBuffer
{
public:
    ByteBuffer() : pointer_(buffer_.begin()) {}
    
    using Iterator = std::vector<uint8_t>::iterator;
    
    // Write data at the current position and increment the pointer
    template<typename InputIt>
    void write(InputIt begin, InputIt end) {
        buffer_.insert(pointer_, begin, end);
        pointer_ += std::distance(begin, end);
    }
    
    inline void write(const uint8_t *data, std::size_t length) {
        write(data, data + length);
    }
    
    // Write big endian data
    template<typename T>
    void writeBigEndian(T t) {
        uint8_t temp[sizeof(T)];
        writeBE<T>(t, temp, temp + sizeof(T));
        write(temp, sizeof(T));
    }
    
    // Write little endian data
    template<typename T>
    void writeLittleEndian(T t) {
        uint8_t temp[sizeof(T)];
        writeLE<T>(t, temp, temp + sizeof(T));
        write(temp, sizeof(T));
    }
    
    // Returns the amount of bytes remaining
    std::size_t remaining() {
        return buffer_.size() - std::distance(buffer_.begin(), pointer_);
    }
    
    // Returns true if there is enough remaining to read the type
    template<typename T>
    inline bool canRead() {
        return remaining() >= sizeof(T);
    }
    
    // Reads data and increments the pointer
    template<typename OutputIt>
    void read(OutputIt begin, OutputIt end) {
        std::size_t toRead = std::distance(begin, end);
        if (toRead > remaining()) {
            throw std::runtime_error(std::string("Not enough bytes available to read (want " + std::to_string(toRead) + ", " + std::to_string(remaining()) + " remaining)"));
        }
        for (; begin != end; ++begin) {
            *begin++ = *pointer_++;
        }
    }
    
    // Reads big endian data
    template<typename T>
    T readBigEndian() {
       if (!canRead<T>()) {
           throw std::runtime_error(std::string("Not enough bytes available to read ") + typeid(T).name());
       }
       auto t = readBE<T>(pointer_, pointer_ + sizeof(T));
       pointer_ += sizeof(T);
       return t;
    }
    
    // Reads little endian data
    template<typename T>
    T readLittleEndian() {
       if (!canRead<T>()) {
           throw std::runtime_error(std::string("Not enough bytes available to read ") + typeid(T).name());
       }
       auto t = readLE<T>(pointer_, pointer_ + sizeof(T));
       pointer_ += sizeof(T);
       return t;
    }
        
    
private:
    std::vector<uint8_t> buffer_;
    Iterator pointer_;
};

#endif // BYTEBUFFER_H
