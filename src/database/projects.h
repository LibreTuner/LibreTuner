#ifndef LIBRETUNER_PROJECTS_H
#define LIBRETUNER_PROJECTS_H

#include <QAbstractItemModel>
#include <QFileSystemWatcher>

#include <lt/project/project.h>

struct TreeItem;

class Projects : public QAbstractItemModel
{
public:
    explicit Projects(QObject * parent = nullptr);
    ~Projects() override;

    /* Adds a project. The project directory must exist in order
     * for the file watcher to work. */
    void addProject(lt::ProjectPtr project);

    QModelIndex index(int row, int column,
                      const QModelIndex & parent) const override;
    QModelIndex parent(const QModelIndex & child) const override;
    int rowCount(const QModelIndex & parent) const override;
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    void refreshRoms(const lt::Project & project);

    QModelIndex projectIndex(const lt::Project & project);

    // Returns the index of the ROM directory with path `romsPath`
    QModelIndex romsIndex(const QString & romsPath);

    QModelIndex tunesIndex(const QString & tunesPath);

private:
    TreeItem * root_;
    QFileSystemWatcher romsWatcher_;
    QFileSystemWatcher tunesWatcher_;

    void refreshRoms(const QModelIndex & index);
    void refreshTunes(const QModelIndex & index);

private slots:
    void romsDirectoryChanged(const QString & path);
    void tunesDirectoryChanged(const QString & path);
};

Q_DECLARE_METATYPE(lt::ProjectPtr)
Q_DECLARE_METATYPE(lt::Rom::MetaData)
Q_DECLARE_METATYPE(lt::Tune::MetaData)

#endif // LIBRETUNER_PROJECTS_H
