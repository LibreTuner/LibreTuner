#include "j2534caninterface.h"

namespace j2534 {

std::shared_ptr<Can> Can::create(Device &device, uint32_t baudrate)
{
    return std::make_shared<Can>(std::move(device), baudrate);
}

Can::Can(Device &device, uint32_t baudrate) : channel_(device.connect(Protocol::CAN, 0, baudrate))
{

}


}
