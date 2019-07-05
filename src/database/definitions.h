#ifndef DB_DEFINITION_H
#define DB_DEFINITION_H

#include <QAbstractItemModel>

#include <memory>

namespace lt
{
class Platforms;
class Model;
using ModelPtr = std::shared_ptr<Model>;
class Platform;
using PlatformPtr = std::shared_ptr<Platform>;
} // namespace lt

class PlatformsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PlatformsModel(lt::Platforms * platforms = nullptr,
                            QObject * parent = nullptr);
    /*
    PlatformsVec::iterator begin() { return platforms_.begin(); }
    PlatformsVec::iterator end() { return platforms_.end(); }
    PlatformsVec::const_iterator cbegin() const { return platforms_.cbegin(); }
    PlatformsVec::const_iterator cend() const { return platforms_.cend(); }*/

    inline void setPlatforms(lt::Platforms * platforms) noexcept
    {
        platforms_ = platforms;
    }

private:
    lt::Platforms * platforms_;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column,
                              const QModelIndex & parent) const override;
    virtual QModelIndex parent(const QModelIndex & child) const override;
    virtual int rowCount(const QModelIndex & parent) const override;
    virtual int columnCount(const QModelIndex & parent) const override;
    virtual QVariant data(const QModelIndex & index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role) const override;
};

Q_DECLARE_METATYPE(lt::ModelPtr)
Q_DECLARE_METATYPE(lt::PlatformPtr)

#endif
