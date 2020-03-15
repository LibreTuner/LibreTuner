#include "TableView.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <QSplitter>

class TableDelegate : public QStyledItemDelegate
{
public:
    explicit TableDelegate(QObject * parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override
    {
        QVariant background = index.data(Qt::BackgroundColorRole);
        if (background.isValid())
        {
            painter->fillRect(option.rect, background.value<QColor>());
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
};

TableView::~TableView()
{

}

TableView::TableView(lt::Table && table, QWidget * parent) : model_(std::move(table))
{
    auto * vLayout = new QVBoxLayout;
    labelX_ = new QLabel("X-Axis");
    labelX_->setAlignment(Qt::AlignCenter);
    vLayout->addWidget(labelX_);

    auto * hLayout = new QHBoxLayout;

    view_ = new QTableView(this);
    view_->setModel(&model_);
    view_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    view_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    view_->horizontalHeader()->setDefaultSectionSize(70);
    view_->horizontalHeader()->setMinimumSectionSize(70);
    view_->horizontalHeader()->setStretchLastSection(true);

    // view_->setItemDelegate(new TableDelegate);

    labelY_ = new VerticalLabel("Y-Axis");
    labelY_->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(labelY_);
    hLayout->addWidget(view_);

    vLayout->addLayout(hLayout);

    auto * tableWidget = new QWidget;
    tableWidget->setLayout(vLayout);

    auto * splitter = new QSplitter(Qt::Vertical);
    splitter->addWidget(tableWidget);

    layout_ = new QVBoxLayout;
    layout_->addWidget(splitter);
    setLayout(layout_);

    connect(&model_, &TableModel::modelReset, this, &TableView::axesChanged);
    axesChanged();
}

void TableView::axesChanged()
{
    const lt::Table & table = model_.table();

    if (table.xAxis())
    {
        labelX_->setText(QString::fromStdString(table.xAxis()->name()));
        labelX_->setVisible(true);
        // view_->horizontalHeader()->setVisible(true);
    }
    else
    {
        labelX_->setVisible(false);
        // view_->horizontalHeader()->setVisible(false);
    }

    if (table.yAxis())
    {
        labelY_->setText(QString::fromStdString(table.yAxis()->name()));
        labelY_->setVisible(true);
        // view_->verticalHeader()->setVisible(true);
    }
    else
    {
        labelY_->setVisible(false);
        // view_->verticalHeader()->setVisible(false);
    }
}

void TableView::setTable(lt::Table && table)
{
    model_.setTable(std::move(table));
}
