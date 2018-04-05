#include "tuneeditor.h"
#include "ui_tuneeditor.h"
#include "tune.h"
#include "table.h"
#include "tablegroup.h"
#include "tabledefinitions.h"

#include <utility>
#include <vector>
#include <cassert>

#include <QTreeWidget>


TuneEditor::TuneEditor(TuneDataPtr tune, QWidget* parent) : QMainWindow(parent), ui(new Ui::TuneEditor), tune_(tune)
{
    assert(tune);
    
    ui->setupUi(this);
    
    std::vector<std::pair<TableCategory, QTreeWidgetItem*> > categories_;
    
    for (int i = 0; i < tune_->romData()->definitions()->count(); ++i)
    {
        const TableDefinition *def = tune_->romData()->definitions()->at(i);
        
        QTreeWidgetItem *par = nullptr;
        
        for (auto &cat : categories_)
        {
            if (cat.first == def->category())
            {
                par = cat.second;
                break;
            }
        }
        
        if (par == nullptr)
        {
            par = new QTreeWidgetItem(ui->treeTables);
            switch (def->category())
            {
                case TCAT_LIMITER:
                    par->setText(0, tr("Limiters"));
                    break;
                case TCAT_MISC:
                    par->setText(0, tr("Miscellaneous"));
                    break;
            }
            
            categories_.push_back(std::make_pair(def->category(), par));
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(par);
        item->setText(0, QString::fromStdString(def->name()));
        item->setData(0, Qt::UserRole, QVariant(i));
    }

    
    setWindowFlag(Qt::Window);
}



void TuneEditor::on_treeTables_itemActivated(QTreeWidgetItem* item, int column)
{
    QVariant data = item->data(0, Qt::UserRole);
    bool ok;
    int index = data.toInt(&ok);
    if (!ok)
    {
        return;
    }
    
    currentTable_ = tune_->tables()->get(index);
    
    ui->tableEdit->setModel(currentTable_.get());
    ui->tableEdit->resizeColumnsToContents();
    ui->tableEdit->resizeRowsToContents();
}
