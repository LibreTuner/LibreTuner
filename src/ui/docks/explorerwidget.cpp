#include "explorerwidget.h"

#include <QVBoxLayout>
#include <QTreeView>
#include <QVector>
#include <QAbstractItemModel>
#include <QFileIconProvider>

#include <lt/project/project.h>
#include "../../database/projects.h"

#include <memory>

ExplorerWidget::ExplorerWidget(QWidget * parent) : QWidget(parent)
{
    tree_ = new QTreeView;
    tree_->setHeaderHidden(true);

    auto * layout = new QVBoxLayout;
    layout->addWidget(tree_);

    setLayout(layout);

    tree_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree_, &QTreeView::customContextMenuRequested, [this](const QPoint & point)
    {
        QModelIndex index = tree_->indexAt(point);

        QVariant data = index.data(Qt::UserRole);
        if (data.canConvert<lt::Project*>())
            menu_.setProject(data.value<lt::Project*>());
        else
            menu_.setProject(nullptr);

        menu_.exec(tree_->viewport()->mapToGlobal(point));
    });
}

void ExplorerWidget::setModel(QAbstractItemModel * model) {
    tree_->setModel(model);
}

ExplorerMenu::ExplorerMenu(QWidget * parent) : QMenu(parent) {
    actionNewProject_ = new QAction(tr("New Project"), this);
    actionDownloadRom_ = new QAction(style()->standardIcon(QStyle::SP_DriveNetIcon),
                                     tr("Download ROM"), this);

    addAction(actionNewProject_);
    addAction(actionDownloadRom_);
}

void ExplorerMenu::setProject(lt::Project * project)
{
    project_ = project;
    actionDownloadRom_->setEnabled(project_ != nullptr);
}
