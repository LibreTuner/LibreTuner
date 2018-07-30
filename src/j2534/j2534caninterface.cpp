#include "j2534caninterface.h"

namespace j2534 {

std::shared_ptr<Can> Can::create(const DevicePtr &device, uint32_t baudrate)
{
    return std::make_shared<Can>(device, baudrate);
}

Can::Can(const DevicePtr &device, uint32_t baudrate) : channel_(device->connect(Protocol::CAN, 0, baudrate))
{

}

bool Can::valid()
{
    return channel_.valid();
}


}
