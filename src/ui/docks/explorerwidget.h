#ifndef LIBRETUNER_EXPLORERWIDGET_H
#define LIBRETUNER_EXPLORERWIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include <QMenu>

#include <lt/project/project.h>

class QTreeView;
class QAbstractItemModel;

class ExplorerMenu : public QMenu
{
public:
    explicit ExplorerMenu(QWidget * parent = nullptr);

    void setProject(lt::Project * project);

    QAction * actionNewProject() noexcept { return actionNewProject_; }
    QAction * actionDownloadRom() noexcept { return actionDownloadRom_; }

private:
    QAction * actionNewProject_;
    QAction * actionDownloadRom_;

    lt::Project * project_{nullptr};
};

class ExplorerWidget : public QWidget
{
public:
    explicit ExplorerWidget(QWidget * parent = nullptr);

    void setModel(QAbstractItemModel * model);

    ExplorerMenu & menu() noexcept { return menu_; }

private:
    QTreeView * tree_;
    QFileIconProvider iconProvider_;
    ExplorerMenu menu_;
};

#endif // LIBRETUNER_EXPLORERWIDGET_H
