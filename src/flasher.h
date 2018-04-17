#ifndef FLASHER_H
#define FLASHER_H

#include <memory>
#include <string>

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

class CanInterface;
typedef std::shared_ptr<CanInterface> CanInterfacePtr;

class Flashable;



enum FlashMode
{
    FLASH_T1, // Uses a CAN interface. Supported: Mazdaspeed 6
    
};

/**
 * An interface for flashing ROMs
 */
class FlashInterface
{
public:
    
};

class Flasher
{
public:
    class Callbacks
    {
    public:
        virtual void onProgress(double percent) =0;
        
        virtual void onError(const std::string &error) =0;
    };
    
    /* Creates a T1 flash interface */
    static FlasherPtr createT1(CanInterfacePtr can);
    
    void flash(const Flashable &f);
};

#endif // FLASHER_H
