#ifndef DB_DEFINITION_H
#define DB_DEFINITION_H

#include "lt/definition/platform.h"

#include <QAbstractItemModel>

#include <filesystem>

class Definitions : public QAbstractItemModel {
    Q_OBJECT
public:
    using PlatformsVec = std::vector<lt::PlatformPtr>;

    explicit Definitions(std::filesystem::path path) : path_(std::move(path)) {}
    Definitions() = default;

    Definitions(const Definitions &) = delete;
    Definitions &operator=(const Definitions &) = delete;

    inline void setPath(const std::filesystem::path &path) noexcept {
        path_ = path;
    }
    inline const std::filesystem::path &path() const noexcept { return path_; }

    // Loads all definitions. Clears previous.
    void load();

    void loadPlatform(const std::filesystem::path &path);

    PlatformsVec::iterator begin() { return platforms_.begin(); }
    PlatformsVec::iterator end() { return platforms_.end(); }
    PlatformsVec::const_iterator cbegin() const { return platforms_.cbegin(); }
    PlatformsVec::const_iterator cend() const { return platforms_.cend(); }

    // Returns a pointer to the platform with id `id`. Returns
    // nullptr if the platform does not exist.
    lt::PlatformPtr fromId(const std::string &id) const noexcept;

    // Returns the total amount of platform definitions
    inline std::size_t count() const noexcept { return platforms_.size(); }

    // Returns first valid platform
    lt::PlatformPtr first() const noexcept;

private:
    PlatformsVec platforms_;

    std::filesystem::path path_;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;
};

Q_DECLARE_METATYPE(lt::PlatformPtr)
Q_DECLARE_METATYPE(lt::ModelPtr)

#endif
