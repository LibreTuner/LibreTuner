#include "tableswidget.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QTreeWidget>
#include <QVBoxLayout>

TablesWidget::TablesWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    view_ = new QTreeWidget(this);
    view_->header()->setVisible(false);

    layout->addWidget(view_);
    setLayout(layout);

    connect(view_, &QTreeWidget::itemActivated,
            [this](const QTreeWidgetItem *item, int column) {
                emit activated(item->data(0, Qt::UserRole)
                                   .value<const lt::ModelTable *>());
            });
}

void TablesWidget::setModel(const lt::Model &model) {
    view_->clear();
    std::vector<std::pair<std::string, QTreeWidgetItem *>> categories_;

    for (const lt::ModelTable &def : model.tables) {
        if (def.table == nullptr) { continue; }
        QTreeWidgetItem *par = nullptr;

        const lt::TableDefinition *table = def.table;

        for (auto &cat : categories_) {
            if (cat.first == table->category) {
                par = cat.second;
                break;
            }
        }

        if (par == nullptr) {
            par = new QTreeWidgetItem(view_);
            par->setText(0, QString::fromStdString(table->category));
            par->setData(0, Qt::UserRole, QVariant(-1));

            categories_.emplace_back(table->category, par);
        }

        auto *item = new QTreeWidgetItem(par);
        item->setText(0, QString::fromStdString(table->name));
        item->setData(0, Qt::UserRole, QVariant::fromValue(&def));
    }
}
