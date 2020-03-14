#ifndef LT_PLATFORM_H
#define LT_PLATFORM_H

#include <filesystem>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../auth/auth.h"
#include "../datalog/pid.h"
#include "../support/types.h"
#include "model.h"
#include "table.h"

namespace lt
{

struct Platform
{
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

    Endianness endianness{Endianness::Big};

    int lastAxisId = 0;
    uint32_t romsize;

    std::unordered_map<std::string, TableDefinition> tables;
    std::vector<Pid> pids;

    std::unordered_map<std::string, AxisDefinition> axes;
    std::vector<std::unique_ptr<Model>> models;
    std::vector<std::regex> vins;

    /* Returns true if the supplied VIN matches any pattern in vins */
    bool matchVin(const std::string & vin) const noexcept;

    /* Searched for a model definition from an id. Returns nullptr
     * if the id does not match a model. */
    Model * findModel(const std::string & id) const noexcept;

    /* Attempts to determine the model of the data. Returns
     * nullptr if no models match. */
    Model * identify(const uint8_t * data, size_t size) const noexcept;

    // Returns the PID with id `id` or nullptr if none exist
    const Pid * getPid(uint32_t id) const noexcept;

    /* Gets the table definition by id. Returns nullptr
     * if the table does not exist. NOTE: If the caller stores the result,
     * it must guarantee that the platform definition lives longer than
     * the table definition. */
    const TableDefinition * getTable(const std::string & id) const noexcept;

    /* Gets the axis definition with id `id`. Returns nullptr
     * if the axis with the id does not exist. See note on `getTable` */
    const AxisDefinition * getAxis(const std::string & id) const noexcept;

    /* Loads platform directory in the format
     * platform_1/    // Platform directory name is unused
     *    main.json    // Platform definition
     *    model1.json  // Model definition
     *    model2.json
     */
    static std::unique_ptr<Platform> loadDirectory(const std::filesystem::path & path);
};

// Loads and stores platforms
class Platforms
{
public:
    /* Loads definitions from a directory. Definitions files
     * should be stored as follows:
     * `path`/
     *    platform_1/    // Platform directory name is unused
     *      main.json    // Platform definition
     *      model1.json  // Model definition
     *      model2.json
     */
    void loadDirectory(const std::filesystem::path & path);

    /* Searches for a platform with id `id`. Returns
     * a null pointer if the search fails. */
    Platform * find(const std::string & id) const noexcept;

    /* Searches for a model by first searching for the platform id `platformId`
     * and then searching for the model `modelId`. Returns nullptr if the
     * search fails. */
    Model * find(const std::string & platformId, const std::string & modelId) const noexcept;

    inline std::size_t size() const noexcept { return platforms_.size(); }

    /* Returns the first platform in the database. Returns nullptr if
     * the database is empty. */
    Platform * first() const noexcept;

    /* Gets the platform at index `index`. There is no guarantee
     * that a platform will always have the same index. Bounds are NOT
     * checked. The caller must guarantee that the index is valid.
     * (see `size()`), otherwise UB will occur. */
    Platform * at(int index) const { return platforms_[index].get(); }

    virtual ~Platforms() = default;

private:
    std::vector<std::unique_ptr<Platform>> platforms_;
};

} // namespace lt

#endif
