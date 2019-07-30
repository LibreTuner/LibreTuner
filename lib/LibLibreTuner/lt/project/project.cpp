#include <utility>

#include "project.h"

#include <cassert>
#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace lt
{

RomPtr Project::getRom(const std::string & filename)
{
    // Search the cache
    if (auto it = cache_.find(filename); it != cache_.end())
    {
        // Check if the pointer has expired
        if (auto rom = it->second.lock())
            return rom;
    }

    std::ifstream file(romsDir_ / filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
        return RomPtr();

    // Deserialize ROM with cereal
    cereal::BinaryInputArchive archive(file);
    Rom::MetaData meta;
    std::vector<uint8_t> data;
    archive(meta, data);

    // Find the model
    ModelPtr model =
        platforms_.find(meta.platform, meta.model);
    if (!model)
        throw std::runtime_error("Unknown platform and rom combination '" +
                                 meta.platform + "' and '" +
                                 meta.model + "'");
    auto rom = std::make_shared<Rom>(model);
    rom->setPath(romsDir_ / filename);
    rom->setName(meta.name);
    rom->setData(std::move(data));
    // Insert into cache
    cache_.emplace(filename, rom);
    return rom;
}

TunePtr Project::loadTune(const std::string & filename)
{
    std::ifstream file(tunesDir_ / filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
        return TunePtr();

    cereal::BinaryInputArchive archive(file);
    Tune::MetaData meta;
    MemoryBuffer data;
    archive(meta, data);

    RomPtr rom = getRom(meta.base);
    if (!rom)
        throw std::runtime_error("unable to find ROM with id '" +
                                 meta.base + "'");

    auto tune = std::make_shared<Tune>(rom, std::move(data));
    tune->setPath(tunesDir_ / filename);
    tune->setName(meta.name);
    return tune;
}

const fs::path & Project::tunesDirectory() const noexcept { return tunesDir_; }

const fs::path & Project::romsDirectory() const noexcept { return romsDir_; }

Project::Project(fs::path base, const Platforms & platforms)
    : path_(base), tunesDir_(base / "tunes"), romsDir_(base / "roms"),
      platforms_(std::move(platforms))
{
}

template <typename MetaData>
std::vector<MetaData> getMetaData(fs::path & dir, bool requiresExtension,
                                  const std::string & extension)
{
    std::vector<MetaData> metadata;
    for (const auto & entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file() ||
            (requiresExtension && entry.path().extension() != extension))
            continue;

        std::ifstream file(entry.path(), std::ios::binary | std::ios::in);
        if (!file.is_open())
            continue; // TODO: Log this

        cereal::BinaryInputArchive ar(file);
        MetaData md;
        try
        {
            ar(md);
        }
        catch (const std::runtime_error & err)
        {
            // TODO: Log exception
        }
        md.path = entry.path();
        metadata.emplace_back(std::move(md));
    }
    return metadata;
}

std::vector<Rom::MetaData> Project::queryRoms()
{
    if (!fs::exists(romsDir_))
        return std::vector<Rom::MetaData>();
    return getMetaData<Rom::MetaData>(romsDir_, enforceExtensions_,
                                      Rom::extension);
}

std::vector<Tune::MetaData> Project::queryTunes()
{
    if (!fs::exists(tunesDir_))
        return std::vector<Tune::MetaData>();
    return getMetaData<Tune::MetaData>(tunesDir_, enforceExtensions_,
                                       Tune::extension);
}

TunePtr Project::createTune(RomPtr base, const std::string & name)
{
    assert(base);

    auto tune = std::make_shared<Tune>(std::move(base));
    tune->setName(name);
    tune->setPath(generateTunePath(name));
    // tunes_.emplace_back(tune);
    return tune;
}

void Project::makeDirectories()
{
    fs::create_directories(romsDirectory());
    fs::create_directories(tunesDirectory());
    fs::create_directories(logsDirectory());
}

std::filesystem::path Project::logsDirectory() const noexcept
{
    return path_ / "logs";
}

inline fs::path generatePath(const fs::path & dir, std::string && id,
                             const std::string & extension)
{
    // Remove whitespace and convert to lowercase
    lt::remove_whitespace(id);
    lt::lowercase_string(id);

    if (!fs::exists(dir / (id + extension)))
        return dir / (id + extension);

    for (int i = 0;; ++i)
    {
        std::string newId = id + "_" + std::to_string(i) + extension;
        if (!fs::exists(dir / newId))
            return dir / newId;
    }
}

fs::path Project::generateRomPath(std::string name)
{
    return generatePath(romsDir_, std::move(name), Rom::extension);
}

fs::path Project::generateTunePath(std::string name)
{
    return generatePath(tunesDir_, std::move(name), Tune::extension);
}

void Project::save() const
{
    fs::path path = path_ / config_filename;
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file '" +
                                 path.string() + "' for writing.");

    json j;
    j["name"] = name_;
    file << j;
}

void Project::load()
{
    fs::path path = path_ / config_filename;
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file '" +
                                 path.string() + "' for reading.");

    json j;
    file >> j;
    j.at("name").get_to(name_);
}

RomPtr Project::createRom(const std::string & name, lt::ModelPtr model)
{
    auto rom = std::make_shared<lt::Rom>(model);
    rom->setName(name);
    rom->setPath(generateRomPath(name));

    cache_.emplace(rom->path().string(), rom);
    return rom;
}

RomPtr Project::importRom(const std::string & name,
                          const std::filesystem::path & path,
                          lt::PlatformPtr platform)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("failed to open ROM file '" + path.string() +
                                 "'");

    // Get file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate buffer
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char *>(buffer.data()), size);
    file.close();

    // Identify model
    lt::ModelPtr model = platform->identify(
        reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    if (!model)
        throw std::runtime_error(
            "failed to identify model from ROM data for platform '" +
            platform->name + "'");

    lt::RomPtr rom = createRom(name, model);
    rom->setData(std::move(buffer));
    return rom;
}

bool Project::deleteRom(const std::string & filename)
{
    cache_.erase(filename);
    return fs::remove(romsDir_ / filename);
}

bool Project::deleteTune(const std::string & filename)
{
    return fs::remove(tunesDir_ / filename);
}

} // namespace lt

CEREAL_CLASS_VERSION(lt::Project, 1)
