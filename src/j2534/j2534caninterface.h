#ifndef J2534CANINTERFACE_H
#define J2534CANINTERFACE_H

#include <memory>

#include "protocols/caninterface.h"
#include "j2534.h"


namespace j2534 {

class Can : public CanInterface
{
public:
    // Attempts to open a J2534 channel for CAN. May throw an exception
    std::shared_ptr<Can> create(j2534::Device &device, uint32_t baudrate = 500000);

    Can(j2534::Device &device, uint32_t baudrate = 500000);

    // CanInterface interface
public:
    virtual void send(const CanMessage &message) override;
    virtual bool valid() override;
    virtual void start() override;

private:
    j2534::Channel channel_;
};

}
#endif // J2534CANINTERFACE_H
