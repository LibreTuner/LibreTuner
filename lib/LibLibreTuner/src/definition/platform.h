#ifndef LT_PLATFORM_H
#define LT_PLATFORM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

#include "support/types.h"
#include "auth/auth.h"
#include "table.h"
#include "model.h"
#include "pid.h"

namespace lt {

struct Platform {
    std::string name;
    std::string id;

    std::string downloadMode;
    std::string flashMode;
    uint32_t baudrate{500000};
    std::string logMode;

    auth::Options downloadAuthOptions;
    auth::Options flashAuthOptions;

    /* Server ID for ISO-TP reqeusts */
    unsigned serverId{0x7e0};

    /* Flash region */
    size_t flashOffset, flashSize;

    Endianness endianness;

    int lastAxisId = 0;
    uint32_t romsize;

    std::vector<TableDefinition> tables;
    std::vector<Pid> pids;
    // axes MUST NOT be added to after initialization
    std::unordered_map<std::string, AxisDefinition> axes;
    std::vector<ModelPtr> models;
    std::vector<std::regex> vins;

    /* Returns true if the supplied VIN matches any pattern in vins */
    bool matchVin(const std::string &vin);

    /* Searched for a model definition from an id. Returns nullptr
     * if the id does not match a model. */
    ModelPtr findModel(const std::string &id);

    /* Attempts to determine the model of the data. Returns
     * nullptr if no models match. */
    ModelPtr identify(const uint8_t *data, size_t size);

    // Returns the PID with id `id` or nullptr if none exist
    Pid *getPid(uint32_t id);
};

} // namespace lt

#endif