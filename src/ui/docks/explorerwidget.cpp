#include "explorerwidget.h"

#include <QAbstractItemModel>
#include <QFileIconProvider>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector>

#include "../../database/projects.h"
#include "../downloadwindow.h"
#include <lt/project/project.h>

#include <memory>

ExplorerWidget::ExplorerWidget(QWidget * parent) : QWidget(parent)
{
    tree_ = new QTreeView;
    tree_->setHeaderHidden(true);

    auto * layout = new QVBoxLayout;
    layout->addWidget(tree_);

    setLayout(layout);

    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree_, &QTreeView::customContextMenuRequested,
            [this](const QPoint & point) {
                QModelIndex index = tree_->indexAt(point);

                QVariant data = index.data(Qt::UserRole);
                if (data.canConvert<lt::ProjectPtr>())
                    menu_.setProject(data.value<lt::ProjectPtr>());
                else
                    menu_.setProject(nullptr);

                menu_.exec(tree_->viewport()->mapToGlobal(point));
            });
}

void ExplorerWidget::setModel(QAbstractItemModel * model)
{
    tree_->setModel(model);
}

ExplorerMenu::ExplorerMenu(QWidget * parent) : QMenu(parent)
{
    actionNewProject_ = new QAction(QIcon(":/icons/new_file.svg"), tr("New Project"), this);
    actionDownloadRom_ =
        new QAction(QIcon(":/icons/download.svg"),
                    tr("Download ROM"), this);

    addAction(actionNewProject_);
    addAction(actionDownloadRom_);

    connect(actionDownloadRom_, &QAction::triggered, [this]()
    {
        DownloadWindow dlg(project_);
        dlg.setModal(true);
        dlg.exec();
    });
}

void ExplorerMenu::setProject(lt::ProjectPtr project)
{
    project_ = std::move(project);
    actionDownloadRom_->setEnabled(!!project_);
}