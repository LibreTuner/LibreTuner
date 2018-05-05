#ifndef FLASHABLE_H
#define FLASHABLE_H

#include <vector>
#include <memory>

class TuneData;
typedef std::shared_ptr<TuneData> TuneDataPtr;

/**
 * A representation of data able to be flashed.
 */
class Flashable
{
public:
    Flashable(TuneDataPtr tune);
    
    bool valid() const
    {
        return valid_;
    }
    
    size_t size() const
    {
        return data_.size();
    }
    
    const uint8_t *data() const
    {
        return data_.data();
    }
    
private:
    std::vector<uint8_t> data_;
    bool valid_;
};

#endif // FLASHABLE_H
