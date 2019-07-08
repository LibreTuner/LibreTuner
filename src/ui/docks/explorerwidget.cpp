#include "explorerwidget.h"

#include <QVBoxLayout>
#include <QTreeView>
#include <QVector>
#include <QAbstractItemModel>
#include <QFileIconProvider>

#include <lt/project/project.h>

#include <memory>

ExplorerWidget::ExplorerWidget(QWidget * parent) : QWidget(parent)
{
    tree_ = new QTreeView;
    tree_->setHeaderHidden(true);

    auto * layout = new QVBoxLayout;
    layout->addWidget(tree_);

    setLayout(layout);
}

void ExplorerWidget::setModel(QAbstractItemModel * model) {
    tree_->setModel(model);
}
