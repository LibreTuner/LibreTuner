#include "editorwidget.h"

#include "verticallabel.h"

#include <QTableView>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;

    labelX_ = new QLabel("X-Axis");
    labelX_->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelX_);

    QHBoxLayout *hLayout = new QHBoxLayout;

    view_ = new QTableView(this);
    view_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    view_->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    view_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);


    labelY_ = new VerticalLabel("Y-Axis");
    labelY_->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(labelY_);
    hLayout->addWidget(view_);

    layout->addLayout(hLayout);
    setLayout(layout);
}

void EditorWidget::setModel(QAbstractItemModel *model)
{
    view_->setModel(model);
}
