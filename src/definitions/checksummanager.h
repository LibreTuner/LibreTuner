#ifndef CHECKSUMMANAGER_H
#define CHECKSUMMANAGER_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class Checksum;
typedef std::shared_ptr<Checksum> ChecksumPtr;

class Checksum
{
public:
    Checksum(uint32_t offset, uint32_t size, uint32_t target) : offset_(offset), size_(size), target_(target) {}
    
    /* Adds a region modifiable for checksum computation */
    void addModifiable(uint32_t offset, uint32_t size);
    
    /* Corrects the checksum for the data using modifiable sections.
     * Returns (false, errmsg) on failure and (true, "") on success. */
    virtual std::pair<bool, std::string> correct (uint8_t *data, size_t length) const =0;
    
    /* Returns the computed checksum. If length is too small,
     * returns 0 and sets ok to false.*/
    virtual uint32_t compute(uint8_t *data, size_t length, bool *ok = nullptr) const =0;
    
protected:
    uint32_t offset_;
    uint32_t size_;
    uint32_t target_;
    
    std::vector<std::pair<uint32_t, uint32_t> > modifiable_;
};



/* Basic type checksum */
class ChecksumBasic : public Checksum
{
public:
    ChecksumBasic(uint32_t offset, uint32_t size, uint32_t target) : Checksum(offset, size, target) {}
    
    uint32_t compute(uint8_t * data, std::size_t length, bool *ok = nullptr) const override;
    
    std::pair<bool, std::string> correct(uint8_t *data, std::size_t length) const override;
};



/**
 * Manages ECU checksums
 */
class ChecksumManager
{
public:
    /* Adds a basic type checksum */
    ChecksumBasic *addBasic(uint32_t offset, uint32_t size, uint32_t target);
    
    /* Corrects the checksums for the data using modifiable sections.
     * Returns (false, errmsg) on failure and (true, "") on success. */
    std::pair<bool, std::string> correct(uint8_t *data, size_t length);
    
private:
    std::vector<ChecksumPtr> checksums_;
};

#endif // CHECKSUMMANAGER_H
