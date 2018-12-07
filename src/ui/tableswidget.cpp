#include "tableswidget.h"

#include <QTreeView>
#include <QAbstractItemModel>
#include <QVBoxLayout>

TablesWidget::TablesWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    view_ = new QTreeView(this);

    layout->addWidget(view_);
    setLayout(layout);
}



void TablesWidget::setModel(QAbstractItemModel *model)
{
    view_->setModel(model);
}
