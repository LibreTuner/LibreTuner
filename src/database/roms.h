#ifndef DB_ROMS_H
#define DB_ROMS_H

#include <filesystem>
#include <memory>

#include <QAbstractItemModel>

namespace lt {
struct Platform;
using PlatformPtr = std::shared_ptr<Platform>;

struct Model;
using ModelPtr = std::shared_ptr<Model>;

class Rom;
using RomPtr = std::shared_ptr<Rom>;
} // namespace lt

struct RomMeta {
    std::string id;
    std::string name;
    lt::ModelPtr model;
};

class Definitions;

/**
 * Manages ROM files and metadata
 */
class Roms : public QAbstractItemModel {
public:
    explicit Roms(const Definitions &definitions) : definitions_(definitions) {}

    Roms(const Roms &) = delete;
    Roms &operator=(const Roms &) = delete;

    /* Loads rom list and metadata. */
    void loadRoms();

    /* Saves rom list and metadata */
    void saveRom(const RomMeta &meta);
    
    /* Adds rom and saves metadata */
    void addRom(const lt::RomPtr &rom);
    
    /* Adds rom by data. Performs model identification */
    void addRom(const lt::Platform &platform, const std::string &id, const std::string &name, const uint8_t *data, const std::size_t size);

    lt::RomPtr openRom(const RomMeta &meta) const;
    
    void saveRom(const lt::RomPtr &rom);

    /* Returns the ROM with id or nullptr if the ROM does
     * not exist. Be careful not to store this reference
     * as ROMs can be added or removed. */
    RomMeta *fromId(const std::string &id) noexcept;

    const RomMeta *fromId(const std::string &id) const noexcept;

    inline std::size_t count() const noexcept { return roms_.size(); }

    inline void setPath(const std::filesystem::path &path) noexcept {
        path_ = path;
    }
    inline const std::filesystem::path &path() const noexcept { return path_; }

    using RomVector = std::vector<RomMeta>;
    RomVector::iterator begin() noexcept { return roms_.begin(); }
    RomVector::iterator end() noexcept { return roms_.end(); }
    RomVector::const_iterator cbegin() const noexcept { return roms_.cbegin(); }
    RomVector::const_iterator cend() const noexcept { return roms_.cend(); }

private:
    std::vector<RomMeta> roms_;
    std::size_t nextId_{};

    const Definitions &definitions_;

    // Creates directory if it does not exist
    void createPath();

    RomMeta loadRom(const std::filesystem::path &path);

    std::filesystem::path path_;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif
