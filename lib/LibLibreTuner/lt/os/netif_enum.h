#ifndef LIBRETUNER_NETIF_ENUM_H
#define LIBRETUNER_NETIF_ENUM_H

#include <string>
#include <vector>

namespace lt {

// Returns a list of network interfaces. If `family` is not 0, it
// it used to filter interfaces by family.
std::vector<std::string> enumerateNetworkInterfaces(int family = 0);

}

#endif // LIBRETUNER_NETIF_ENUM_H
