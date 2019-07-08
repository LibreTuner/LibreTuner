#include "projects.h"

#include <QFileIconProvider>

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

class ProjectItem : public TreeItem
{
public:
    explicit ProjectItem(std::shared_ptr<lt::Project> project,
                         TreeItem * parent = nullptr)
        : TreeItem(parent), project_(std::move(project))
    {
        roms_ = new DirectoryItem("ROMs", this);
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

        return QVariant();
    }

private:
    lt::ProjectPtr project_;

    TreeItem * roms_;
    TreeItem * tunes_;
    TreeItem * logs_;
};

Projects::Projects(QObject * parent) : QAbstractItemModel(parent)
{
    root_ = new TreeItem;
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
    new ProjectItem(std::move(project), root_);
}

QVariant Projects::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    return QVariant();
}

Projects::~Projects()
{
    delete root_;
}
