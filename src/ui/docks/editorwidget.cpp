#include "editorwidget.h"

#include "libretuner.h"

#include "../verticallabel.h"
#include "models/tablemodel.h"

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
    
    view_->horizontalHeader()->setDefaultSectionSize(70);
    view_->horizontalHeader()->setMinimumSectionSize(70);
    view_->horizontalHeader()->setStretchLastSection(true);


    labelY_ = new VerticalLabel("Y-Axis");
    labelY_->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(labelY_);
    hLayout->addWidget(view_);

    layout->addLayout(hLayout);
    setLayout(layout);
}



void EditorWidget::setModel(TableModel *model)
{
    if (model_ != nullptr) {
        disconnect(model_, &TableModel::modelReset, this, &EditorWidget::axesChanged);
    }
    model_ = model;
    connect(model_, &TableModel::modelReset, this, &EditorWidget::axesChanged);
    view_->setModel(model);

    axesChanged();
}



void EditorWidget::axesChanged()
{
    if (model_ == nullptr) {
        return;
    }
    lt::Table *table = model_->table();
    if (table == nullptr) {
        return;
    }
    
    if (table->axisX()) {
        labelX_->setText(QString::fromStdString(table->axisX()->name()));
        labelX_->setVisible(true);
        view_->horizontalHeader()->setVisible(true);
    } else {
        labelX_->setVisible(false);
        view_->horizontalHeader()->setVisible(false);
    }
    
    if (table->axisY()) {
        labelY_->setText(QString::fromStdString(table->axisY()->name()));
        labelY_->setVisible(true);
        view_->verticalHeader()->setVisible(true);
    } else {
        labelY_->setVisible(false);
        view_->verticalHeader()->setVisible(false);
    }
}

