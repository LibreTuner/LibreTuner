#ifndef FLASHABLE_H
#define FLASHABLE_H

#include <vector>
#include <memory>

class TuneData;
typedef std::shared_ptr<TuneData> TuneDataPtr;

/**
 * @todo write docs
 */
class Flashable
{
public:
    Flashable(TuneDataPtr tune);
    
    bool valid() const
    {
        return valid_;
    }
    
    
private:
    std::vector<uint8_t> data_;
    bool valid_;
};

#endif // FLASHABLE_H
