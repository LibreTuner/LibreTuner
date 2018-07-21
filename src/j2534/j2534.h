#ifndef J2534_H
#define J2534_H

#include <memory>
#include <string>

#include "datalink.h"

struct J2534Info {
    std::string name;
    // Supported protocols
    DataLinkProtocol protocols;
    // DLL path
    std::string functionLibrary;
};

class J2534
{
public:
    J2534();
};
using J2534Ptr = std::shared_ptr<J2534>;

#endif // J2534_H
