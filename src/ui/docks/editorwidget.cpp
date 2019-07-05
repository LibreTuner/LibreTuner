#include "editorwidget.h"

#include "libretuner.h"

#include "../verticallabel.h"
#include "models/tablemodel.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>

class TableDelegate : public QStyledItemDelegate {
public:
    explicit TableDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QVariant background = index.data(Qt::BackgroundColorRole);
        if (background.isValid()) {
            painter->fillRect(option.rect, background.value<QColor>());
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
};

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent) {
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

    view_->setItemDelegate(new TableDelegate);

    labelY_ = new VerticalLabel("Y-Axis");
    labelY_->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(labelY_);
    hLayout->addWidget(view_);

    layout->addLayout(hLayout);
    setLayout(layout);

    // setStyleSheet("QTableView::item { border: 0; }");
}

void EditorWidget::setModel(TableModel *model) {
    if (model_ != nullptr) {
        disconnect(model_, &TableModel::modelReset, this,
                   &EditorWidget::axesChanged);
    }
    model_ = model;
    connect(model_, &TableModel::modelReset, this, &EditorWidget::axesChanged);
    view_->setModel(model);

    axesChanged();
}

void EditorWidget::axesChanged() {
    if (model_ == nullptr) {
        return;
    }
    lt::Table *table = model_->table();
    if (table == nullptr) {
        return;
    }

    if (table->xAxis()) {
        // labelX_->setText(QString::fromStdString(table->xAxis()->name()));
        labelX_->setVisible(true);
        view_->horizontalHeader()->setVisible(true);
    } else {
        labelX_->setVisible(false);
        view_->horizontalHeader()->setVisible(false);
    }

    if (table->yAxis()) {
        // labelY_->setText(QString::fromStdString(table->axisY()->name()));
        labelY_->setVisible(true);
        view_->verticalHeader()->setVisible(true);
    } else {
        labelY_->setVisible(false);
        view_->verticalHeader()->setVisible(false);
    }
}
