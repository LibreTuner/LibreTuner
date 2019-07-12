#include "projects.h"

#include <QFileIconProvider>
#include <logger.h>

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

class ProjectItem : public TreeItem
{
public:
    explicit ProjectItem(lt::ProjectPtr project, TreeItem * parent = nullptr)
        : TreeItem(parent), project_(std::move(project))
    {
        roms_ = new RomsItem(project_, this);
        tunes_ = new DirectoryItem("Tunes", this);
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
    TreeItem * tunes_;
    TreeItem * logs_;
};

Projects::Projects(QObject * parent) : QAbstractItemModel(parent)
{
    root_ = new TreeItem;

    connect(&romsWatcher_, &QFileSystemWatcher::directoryChanged, this,
            &Projects::romsDirectoryChanged);
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
    if (!romsWatcher_.addPath(
            QString::fromStdString(project->romsDirectory().string())))
        Logger::warning("Failed to add roms path '" +
                        project->romsDirectory().string() +
                        "' to file watcher.");

    beginInsertRows(QModelIndex(), root_->children.size(),
                    root_->children.size());
    new ProjectItem(std::move(project), root_);
    endInsertRows();
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
    auto romsItem = reinterpret_cast<RomItem *>(index.internalPointer());

    // Found the correct project, reset roms
    beginRemoveRows(index, 0, romsItem->children.size());
    qDeleteAll(romsItem->children);
    romsItem->children.clear();
    endRemoveRows();

    // Get all ROM metadata
    auto roms = project->queryRoms();
    beginInsertRows(index, 0, roms.size());
    for (const auto & rom : roms)
    {
        new RomItem(rom, romsItem);
    }
    endInsertRows();
}
