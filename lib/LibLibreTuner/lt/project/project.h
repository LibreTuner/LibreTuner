#ifndef LIBRETUNER_PROJECT_H
#define LIBRETUNER_PROJECT_H

#include "../rom/rom.h"
#include <filesystem>
#include <string>

namespace lt
{
class Project
{
public:
    /* Initializes base path for storage. Tunes and ROMs path are set to
     * '`base`/tunes' and '`base`/roms' respectively. */
    Project(std::filesystem::path base, const Platforms & platforms);

    /* Loads a ROM by filename. If the ROM is cached, it will be returned.
     * Otherwise, the directory is searched and if the ROM cannot
     * be found, RomPtr() is returned. If the ROM was found but deserialization
     * fails, throws an exception. */
    RomPtr getRom(const std::string & filename);

    /* Creates a new blank ROM from `name`. Sets path. */
    RomPtr createRom(const std::string & name,
                     lt::ModelPtr model = lt::ModelPtr());

    /* Deletes ROM by filename. Returns true if the ROM was deleted or
     * false if it could not be found or there is insufficient permission. */
    bool deleteRom(const std::string & filename);

    /* Deletes tune by filename. Returns true if the tune was deleted or
     * false if it could not be found or there is insufficient permission. */
    bool deleteTune(const std::string & filename);

    /* Creates a new ROM from a file containing the raw ROM from an ECU.
     * Throws an exception if the file cannot be opened or the model
     * cannot be determined. */
    RomPtr importRom(const std::string & name,
                     const std::filesystem::path & path,
                     lt::PlatformPtr platform);

    /* Loads tune by id. Returns a null pointer if the path does not
     * exist. Throws an exception if it cannot be deserialized or the base
     * cannot be found. */
    TunePtr loadTune(const std::string & filename);

    /* Saves a tune to a file. Throws an exception if serialization failed or
     * the file couldn't be opened. */
    void saveTune(const Tune & tune, std::filesystem::path & path);

    /* Searches all ROM files and extracts their metadata. Silently ignores
     * invalid ROMs. This is an expensive operation that should not
     * be called often. */
    std::vector<Rom::MetaData> queryRoms();

    /* Searches all tune files and extracts their metadata. Silently ignores
     * invalid tunes. This is an expensive operation that should not
     * be called often. */
    std::vector<Tune::MetaData> queryTunes();

    const std::filesystem::path & tunesDirectory() const noexcept;
    const std::filesystem::path & romsDirectory() const noexcept;

    /* Creates a tune from the base calibration. Generates an
     * id based on the name. */
    TunePtr createTune(RomPtr base, const std::string & name);

    // Getters
    inline const std::string & name() const noexcept { return name_; }

    inline void setName(std::string name) noexcept { name_ = std::move(name); }

    // Saves project configuration. Does NOT save tunes or ROMs.
    void save() const;

    // Loads project configuration
    void load();

    // Creates all project directories
    void makeDirectories();

    // Returns the project's path
    const std::filesystem::path & path() const noexcept { return path_; }

    std::filesystem::path logsDirectory() const noexcept;

private:
    // Project directory
    std::filesystem::path path_;

    std::filesystem::path tunesDir_;
    std::filesystem::path romsDir_;

    // Caches loaded ROMs
    std::unordered_map<std::string, WeakRomPtr> cache_;
    const Platforms & platforms_;

    /* If true, tunes and ROMs must have the proper extension
     * to be loaded. */
    bool enforceExtensions_{true};

    std::string name_;

    // Logs

    // Generates the next available id based on the name
    std::filesystem::path generateRomPath(std::string name);
    std::filesystem::path generateTunePath(std::string name);
};
using ProjectPtr = std::shared_ptr<Project>;

} // namespace lt

#endif // LIBRETUNER_PROJECT_H
