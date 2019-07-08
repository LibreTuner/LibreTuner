#include <utility>

#include "project.h"

#include <cassert>
#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace fs = std::filesystem;

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
    RomConstruct construct;
    archive(construct);

    // Find the model
    ModelPtr model =
        platforms_.find(construct.meta.platform, construct.meta.model);
    if (!model)
        throw std::runtime_error("Unknown platform and rom combination '" +
                                 construct.meta.platform + "' and '" +
                                 construct.meta.model + "'");
    auto rom = std::make_shared<Rom>(model);
    rom->setPath(romsDir_ / filename);
    rom->setName(construct.meta.name);
    rom->setData(std::move(construct.data));
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
    TuneConstruct construct;
    archive(construct);

    RomPtr rom = getRom(construct.meta.base);
    if (!rom)
        throw std::runtime_error("unable to find ROM with id '" +
                                 construct.meta.base + "'");

    auto tune = std::make_shared<Tune>(rom);
    tune->setPath(tunesDir_ / filename);
    tune->setName(construct.meta.name);

    for (const TableConstruct & table : construct.tables)
    {
        tune->setTable(table.id, table.data.data(), table.data.size());
    }
    return tune;
}

fs::path Project::tunesDirectory() const noexcept { return tunesDir_; }

fs::path Project::romsDirectory() const noexcept { return romsDir_; }

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
        metadata.emplace_back(std::move(md));
    }
    return metadata;
}

std::vector<Rom::MetaData> Project::queryRoms()
{
    return getMetaData<Rom::MetaData>(romsDir_, enforceExtensions_,
                                      Rom::extension);
}

std::vector<Tune::MetaData> Project::queryTunes()
{
    return getMetaData<Tune::MetaData>(tunesDir_, enforceExtensions_,
                                       Tune::extension);
}

TunePtr Project::createTune(RomPtr base, const std::string & name)
{
    assert(base);

    auto tune = std::make_shared<Tune>(std::move(base));
    tune->setName(name);
    tune->setPath(tunesDir_ / generateTuneId(name));
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

inline std::string generateId(const fs::path & dir, std::string && id,
                              const std::string & extension)
{
    // Remove whitespace and convert to lowercase
    lt::remove_whitespace(id);
    lt::lowercase_string(id);

    if (!fs::exists(dir / (id + extension)))
        return id;

    for (int i = 0;; ++i)
    {
        std::string newId = id + "_" + std::to_string(i) + extension;
        if (!fs::exists(dir / newId))
            return id;
    }
}

std::string Project::generateRomId(std::string name)
{
    return generateId(romsDir_, std::move(name), Rom::extension);
}

std::string Project::generateTuneId(std::string name)
{
    return generateId(tunesDir_, std::move(name), Tune::extension);
}

template <class Archive>
void save(Archive & archive, const lt::Project & project, std::uint32_t const version)
{
    archive(project.name());
}

template <class Archive>
void load(Archive & archive, lt::Project & project, std::uint32_t const version)
{
    std::string name;
    archive(name);
    project.setName(std::move(name));
}

void Project::save() const
{
    std::ofstream file(path_, std::ios::binary | std::ios::out);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file '" + path_.string() + "' for writing.");

    cereal::BinaryOutputArchive ar(file);
    ar(*this);
}

void Project::load()
{
    std::ifstream file(path_, std::ios::binary | std::ios::in);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file '" + path_.string() + "' for reading.");

    cereal::BinaryInputArchive ar(file);
    ar(*this);
}

} // namespace lt

CEREAL_CLASS_VERSION(lt::Project, 1)