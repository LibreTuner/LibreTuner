#include "netif_enum.h"

#include <cstring>
#include <stdexcept>
#include <unordered_set>

#ifdef __linux__
#include <ifaddrs.h>
#endif

namespace lt {
std::vector<std::string> enumerateNetworkInterfaces(int family) {
#ifdef __linux__

    std::unordered_set<std::string> ifaces;

    {
        ifaddrs *addrs, *it;

        if (getifaddrs(&addrs) == -1) {
            throw std::runtime_error(strerror(errno));
        }
        it = addrs;

        while (it != nullptr) {
            if (family == 0 || (it->ifa_addr != nullptr &&
                                it->ifa_addr->sa_family == family)) {
                ifaces.emplace(it->ifa_name);
            }
            it = it->ifa_next;
        }

        freeifaddrs(addrs);
    }

    return std::vector<std::string>(std::make_move_iterator(ifaces.begin()),
                                    std::make_move_iterator(ifaces.end()));
#else
    return std::vector<std::string>();
#endif
}

} // namespace lt