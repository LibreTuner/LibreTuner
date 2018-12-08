#include "tableswidget.h"
#include "definitions/definition.h"

#include <QTreeWidget>
#include <QAbstractItemModel>
#include <QVBoxLayout>

TablesWidget::TablesWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    view_ = new QTreeWidget(this);

    layout->addWidget(view_);
    setLayout(layout);
    
    connect(view_, &QTreeWidget::itemActivated, [this](const QTreeWidgetItem *item, int column) {
        emit activated(item->data(0, Qt::UserRole).toInt());
    });
}



void TablesWidget::setTables(const std::vector<definition::Table> &tables)
{
    view_->clear();
    std::vector<std::pair<std::string, QTreeWidgetItem *>> categories_;

    for (const definition::Table &def : tables) {
        QTreeWidgetItem *par = nullptr;

        for (auto &cat : categories_) {
            if (cat.first == def.category) {
                par = cat.second;
                break;
            }
        }

        if (par == nullptr) {
            par = new QTreeWidgetItem(view_);
            par->setText(0, QString::fromStdString(def.category));

            categories_.emplace_back(def.category, par);
        }

        auto *item = new QTreeWidgetItem(par);
        item->setText(0, QString::fromStdString(def.name));
        item->setData(0, Qt::UserRole, QVariant(def.id));
    }
}
