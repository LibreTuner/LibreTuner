#include "projects.h"

#include <QFileIconProvider>
#include <logger.h>
#include <uiutil.h>

// This class is a mess
struct TreeItem
{
    QVector<TreeItem *> children;
    TreeItem * parent{nullptr};

    explicit TreeItem(TreeItem * parentItem = nullptr) : parent(parentItem)
    {
        if (parent != nullptr)
            parent->children.append(this);
    }

    virtual ~TreeItem()
    {
        // Kill all our children
        qDeleteAll(children);
    }

    virtual QVariant data(int column, int role) const { return QVariant(); }

    TreeItem * child(int index)
    {
        if (index < 0 || index >= children.size())
            return nullptr;
        return children[index];
    }

    int row() const
    {
        if (parent)
            return parent->children.indexOf(const_cast<TreeItem *>(this));
        return 0;
    }
};

class DirectoryItem : public TreeItem
{
public:
    explicit DirectoryItem(QString name, TreeItem * parent = nullptr)
        : TreeItem(parent), name_(std::move(name))
    {
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::DisplayRole)
            return name_;

        if (role == Qt::DecorationRole)
        {
            QFileIconProvider provider;
            return provider.icon(QFileIconProvider::Folder);
        }

        return QVariant();
    }

private:
    QString name_;
};

class RomItem : public TreeItem
{
public:
    explicit RomItem(lt::Rom::MetaData md, TreeItem * parent = nullptr)
        : TreeItem(parent), md_(std::move(md))
    {
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::DisplayRole)
            return QString::fromStdString(md_.name);

        if (role == Qt::DecorationRole)
        {
            QFileIconProvider provider;
            return provider.icon(QFileIconProvider::File);
        }

        if (role == Qt::UserRole)
        {
            return QVariant::fromValue(md_);
        }

        return QVariant();
    }

private:
    lt::Rom::MetaData md_;
};

class TuneItem : public TreeItem
{
public:
    explicit TuneItem(lt::Tune::MetaData md, TreeItem * parent = nullptr)
        : TreeItem(parent), md_(std::move(md))
    {
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::DisplayRole)
            return QString::fromStdString(md_.name);

        if (role == Qt::DecorationRole)
        {
            QFileIconProvider provider;
            return provider.icon(QFileIconProvider::File);
        }

        if (role == Qt::UserRole)
            return QVariant::fromValue(md_);

        return QVariant();
    }

private:
    lt::Tune::MetaData md_;
};

class RomsItem : public DirectoryItem
{
public:
    explicit RomsItem(lt::ProjectPtr project, TreeItem * parent = nullptr)
        : DirectoryItem("ROMs", parent), project_(std::move(project))
    {
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::UserRole)
            return QVariant::fromValue(project_);

        return DirectoryItem::data(column, role);
    }

    lt::ProjectPtr project_;
};

class TunesItem : public DirectoryItem
{
public:
    explicit TunesItem(lt::ProjectPtr project, TreeItem * parent = nullptr)
        : DirectoryItem("Tunes", parent), project_(std::move(project))
    {
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::UserRole)
            return QVariant::fromValue(project_);

        return DirectoryItem::data(column, role);
    }

    lt::ProjectPtr project_;
};

class ProjectItem : public TreeItem
{
public:
    explicit ProjectItem(lt::ProjectPtr project, TreeItem * parent = nullptr)
        : TreeItem(parent), project_(std::move(project))
    {
        roms_ = new RomsItem(project_, this);
        tunes_ = new TunesItem(project_, this);
        logs_ = new DirectoryItem("Logs", this);
    }

    QVariant data(int column, int role) const override
    {
        if (column != 0)
            return QVariant();

        if (role == Qt::DisplayRole)
            return QString::fromStdString(project_->name());

        if (role == Qt::DecorationRole)
        {
            QFileIconProvider provider;
            return provider.icon(QFileIconProvider::Drive);
        }

        if (role == Qt::UserRole)
            return QVariant::fromValue(project_);

        return QVariant();
    }

private:
    lt::ProjectPtr project_;

    RomsItem * roms_;
    TunesItem * tunes_;
    TreeItem * logs_;
};

Projects::Projects(QObject * parent) : QAbstractItemModel(parent)
{
    root_ = new TreeItem;

    connect(&romsWatcher_, &QFileSystemWatcher::directoryChanged, this,
            &Projects::romsDirectoryChanged);
    connect(&tunesWatcher_, &QFileSystemWatcher::directoryChanged, this,
            &::Projects::tunesDirectoryChanged);
}

QModelIndex Projects::index(int row, int column,
                            const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem * parentItem;
    if (!parent.isValid())
        parentItem = root_;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    TreeItem * childItem = parentItem->child(row);
    if (childItem != nullptr)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex Projects::parent(const QModelIndex & child) const
{
    if (!child.isValid())
        return QModelIndex();

    TreeItem * childItem = static_cast<TreeItem *>(child.internalPointer());
    TreeItem * parentItem = childItem->parent;
    if (parentItem == root_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int Projects::rowCount(const QModelIndex & parent) const
{
    TreeItem * parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root_;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    return parentItem->children.size();
}

int Projects::columnCount(const QModelIndex & parent) const { return 1; }

QVariant Projects::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto * item = static_cast<TreeItem *>(index.internalPointer());
    return item->data(index.column(), role);
}

void Projects::addProject(lt::ProjectPtr project)
{
    Logger::debug("Adding project '" + project->name() + "'");
    QString romsDir = QString::fromStdString(project->romsDirectory().string());
    QString tunesDir =
        QString::fromStdString(project->tunesDirectory().string());
    if (!romsWatcher_.addPath(romsDir))
        Logger::warning("Failed to add roms path '" + romsDir.toStdString() +
                        "' to file watcher.");
    if (!tunesWatcher_.addPath(tunesDir))
        Logger::warning("Failed to add tunes path '" + tunesDir.toStdString() +
                        "' to file watcher.");

    beginInsertRows(QModelIndex(), root_->children.size(),
                    root_->children.size());
    new ProjectItem(std::move(project), root_);
    endInsertRows();

    // Force update
    romsDirectoryChanged(romsDir);
    tunesDirectoryChanged(tunesDir);
}

QVariant Projects::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    return QVariant();
}

Projects::~Projects() { delete root_; }

QModelIndex Projects::romsIndex(const QString & romsPath)
{
    std::filesystem::path dir(romsPath.toStdString());
    for (int row = 0; row < root_->children.size(); ++row)
    {
        QVariant data = root_->child(row)->data(0, Qt::UserRole);
        if (data.canConvert<lt::ProjectPtr>())
        {
            auto project = data.value<lt::ProjectPtr>();
            if (project->romsDirectory() == dir)
                return index(0, 0, index(row, 0, QModelIndex()));
        }
    }
    return QModelIndex();
}

QModelIndex Projects::tunesIndex(const QString & tunesPath)
{
    std::filesystem::path dir(tunesPath.toStdString());
    for (int row = 0; row < root_->children.size(); ++row)
    {
        QVariant data = root_->child(row)->data(0, Qt::UserRole);
        if (data.canConvert<lt::ProjectPtr>())
        {
            auto project = data.value<lt::ProjectPtr>();
            if (project->tunesDirectory() == dir)
                return index(1, 0, index(row, 0, QModelIndex()));
        }
    }
    return QModelIndex();
}

void Projects::romsDirectoryChanged(const QString & path)
{
    QModelIndex index = romsIndex(path);
    if (!index.isValid())
        return;

    refreshRoms(index);
}

void Projects::refreshRoms(const QModelIndex & index)
{
    auto project = index.data(Qt::UserRole).value<lt::ProjectPtr>();
    auto romsItem = reinterpret_cast<RomsItem *>(index.internalPointer());

    // Found the correct project, reset roms
    if (!romsItem->children.empty())
    {
        beginRemoveRows(index, 0, romsItem->children.size() - 1);
        qDeleteAll(romsItem->children);
        romsItem->children.clear();
        endRemoveRows();
    }

    // Get all ROM metadatal
    catchWarning([&](){
        auto roms = project->queryRoms();
        if (!roms.empty())
        {
            beginInsertRows(index, 0, roms.size() - 1);
            for (const auto & rom : roms)
            {
                new RomItem(rom, romsItem);
            }
            endInsertRows();
        }
    }, tr("Error querying ROM metadata"));
}

// A copy of the above method (gross, but it's quicker than abstracting it).
void Projects::refreshTunes(const QModelIndex & index)
{
    auto project = index.data(Qt::UserRole).value<lt::ProjectPtr>();
    auto tunesItem = reinterpret_cast<TunesItem *>(index.internalPointer());

    // Found the correct project, reset roms
    if (!tunesItem->children.empty())
    {
        beginRemoveRows(index, 0, tunesItem->children.size() - 1);
        qDeleteAll(tunesItem->children);
        tunesItem->children.clear();
        endRemoveRows();
    }

    // Get all tune metadata
    catchWarning([&](){
        auto tunes = project->queryTunes();
        if (!tunes.empty())
        {
            beginInsertRows(index, 0, tunes.size() - 1);
            for (const auto & tune : tunes)
            {
                new TuneItem(tune, tunesItem);
            }
            endInsertRows();
        }
    }, tr("Error querying tune metadata"));
}

void Projects::tunesDirectoryChanged(const QString & path)
{
    QModelIndex index = tunesIndex(path);
    if (!index.isValid())
        return;

    refreshTunes(index);
}
