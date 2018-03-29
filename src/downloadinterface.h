#ifndef DOWNLOADINTERFACE_H
#define DOWNLOADINTERFACE_H

#include <QString>
#include <QFile>

#include <memory>

#include "rommanager.h"

class CanInterface;

class DownloadInterface {
public:
    enum Type
    {
        TYPE_CAN,
        TYPE_J2534,
    };
    
    virtual ~DownloadInterface() {};
    
    /* Starts downloading. Calls updateProgress if possible.
        * Signals onError if an error occurs. */
    virtual void download() =0;
    
    
    class Callbacks {
    public:
        /* Signals an update in progress. progress is a value between 0 and 100 */
        virtual void updateProgress(float progress) =0;
        
        virtual void downloadError(const QString &error) =0;
        
        /* Called when the ROM has finished downloading. */
        virtual void onCompletion(const uint8_t *data, size_t length) =0;
    };

#ifdef WITH_SOCKETCAN
    /* Creates a socketcan downloader. device is the name of the socketcan interface. */
    static std::shared_ptr<DownloadInterface> createSocketCan(Callbacks *callbacks, const std::string &device, RomType vehicle);
#endif
    
protected:
    Callbacks *callbacks_;
    
    DownloadInterface(Callbacks *callbacks);
};

#endif // DOWNLOADINTERFACE_H
