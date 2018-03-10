#ifndef CANHANDLER_H
#define CANHANDLER_H

#include "canlog.h"
#include "caninterface.h"
#include <memory>

class CanInterface;

/**
 * Handles logging CAN data and sends received CAN data to callbacks
 */
class CanHandler : public CanInterface::Callbacks
{
public:
    CanHandler();
    
    CanLog *log()
    {
        return &log_;
    }
    
    /* Starts a SocketCan interface. Returns false if the interface
     * could not be created. */
    bool startSocketCan(const char *name);
    
    
    /* Callbacks */
    void onRecv(const CanMessage & message) override;
    void onError(CanInterface::CanError error, int err) override;
    
private:
    std::unique_ptr<CanInterface> can_;
    CanLog log_;
};

#endif // CANHANDLER_H
