#ifndef LIBRETUNER_EXPLORERWIDGET_H
#define LIBRETUNER_EXPLORERWIDGET_H

#include <QFileIconProvider>
#include <QMenu>
#include <QWidget>

#include <lt/project/project.h>

class QTreeView;
class QAbstractItemModel;

class ExplorerMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ExplorerMenu(QWidget * parent = nullptr);

    void setProject(lt::ProjectPtr project);
    void setRom(const std::string & filename);
    void setTune(const std::string & filename);

    QAction * actionNewProject() noexcept { return actionNewProject_; }
    QAction * actionDownloadRom() noexcept { return actionDownloadRom_; }
    const lt::ProjectPtr & project() const noexcept { return project_; }

public slots:
    void onDelete();

private:
    QAction * actionNewProject_;
    QAction * actionDownloadRom_;
    QAction * actionImportRom_;
    QAction * actionDelete_;
    QAction * actionCreateTune_;
    QAction * actionDuplicate_;

    lt::ProjectPtr project_;
    std::string romFilename_;
    std::string tuneFilename_;
};

class ExplorerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExplorerWidget(QWidget * parent = nullptr);

    void setModel(QAbstractItemModel * model);

    ExplorerMenu & menu() noexcept { return menu_; }

private slots:
    void showContextMenu(const QPoint & point);
    void onActivated(const QModelIndex & index);

signals:
    void tuneOpened(const lt::TunePtr & tune);

public:
    bool eventFilter(QObject * watched, QEvent * event) override;

private:
    void populateMenu(const QModelIndex & index);

    QTreeView * tree_;
    QFileIconProvider iconProvider_;
    ExplorerMenu menu_;
};

#endif // LIBRETUNER_EXPLORERWIDGET_H
