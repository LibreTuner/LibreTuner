#include "j2534.h"
#include <windows.h>

void J2534::init()
{
    load();
}

J2534Ptr J2534::create(J2534Info &&info)
{
    return std::make_shared<J2534>(std::move(info));
}

void J2534::load()
{
    HINSTANCE dll;
}
