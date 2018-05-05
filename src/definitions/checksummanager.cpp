#include "checksummanager.h"
#include "util.hpp"


void Checksum::addModifiable(uint32_t offset, uint32_t size)
{
    modifiable_.push_back(std::make_pair(offset, size));
}



uint32_t ChecksumBasic::compute(uint8_t* data, std::size_t length, bool* ok) const
{
    if (length < offset_ + size_)
    {
        if (ok != nullptr)
        {
            *ok = false;
        }
        return 0;
    }
    
    uint32_t sum = 0;
    // Add up the big endian int32s
    for (int i = 0; i < length / 4; ++i, data += 4)
    {
        sum += toBEInt32(data);
    }
    
    if (ok != nullptr)
    {
        *ok = true;
    }
    return sum;
}



std::pair<bool, std::string> ChecksumBasic::correct(uint8_t* data, std::size_t length) const
{
    if (length < offset_ + size_)
    {
         return std::make_pair<bool, std::string>(false, "Checksum region exceeds the rom size.");
    }
    
    bool foundMod = false;
    uint32_t modifiableOffset;
    
    // Find a usable modifiable region
    for (auto it = modifiable_.begin(); it != modifiable_.end(); ++it)
    {
        if (it->second >= 4)
        {
            modifiableOffset = it->first;
            foundMod = true;
            break;
        }
    }
    if (!foundMod)
    {
        return std::make_pair<bool, std::string>(false, "Failed to find a usable modifiable region for checksum correction.");
    }
    
    // Zero the region
    writeBEInt32(0, data + offset_ + modifiableOffset);
    
    // compute should never fail after the check above
    uint32_t oSum = compute(data, length);
    
    uint32_t val = target_ - oSum;
    writeBEInt32(val, data + offset_ + modifiableOffset);
    
    // Check if the correction was successful
    if (compute(data, length) != target_)
    {
        return std::make_pair<bool, std::string>(false, "Checksum does not equal target after correction");
    }
    return std::make_pair<bool, std::string>(true, std::string());
}



std::pair<bool, std::string> ChecksumManager::correct(uint8_t* data, size_t length)
{
    for (const ChecksumPtr &checksum : checksums_)
    {
        auto res = checksum->correct(data, length);
        if (!res.first)
        {
            return res;
        }
    }
    return std::make_pair<bool, std::string>(true, std::string());
}



ChecksumBasic *ChecksumManager::addBasic(uint32_t offset, uint32_t size, uint32_t target)
{
    ChecksumPtr checksum = std::make_shared<ChecksumBasic>(offset, size, target);
    
    checksums_.push_back(checksum);
    return reinterpret_cast<ChecksumBasic*>(checksum.get());
}
