#include "overviewwidget.h"

#include "rommanager.h"
#include "definitions/definitionmanager.h"
#include "libretuner.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>

OverviewWidget::OverviewWidget(QWidget* parent) : QWidget(parent)
{
    lineDefinitionCount_ = new QLineEdit;
    lineDefinitionCount_->setReadOnly(true);
    
    lineRomCount_ = new QLineEdit;
    lineRomCount_->setReadOnly(true);
    
    lineDatalinkCount_ = new QLineEdit;
    lineDatalinkCount_->setReadOnly(true);
    
    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Definitions loaded:"), lineDefinitionCount_);
    form->addRow(tr("ROMs loaded:"), lineRomCount_);
    form->addRow(tr("Datalinks detected:"), lineDatalinkCount_);
    
    lineRomCount_->setText(QString::number(RomStore::get()->count()));
    lineDefinitionCount_->setText(QString::number(DefinitionManager::get()->rowCount(QModelIndex())));
    lineDatalinkCount_->setText(QString::number(LT()->links().count()));
    
    auto *layout = new QVBoxLayout;
    layout->addLayout(form);
    
    setLayout(layout);
}
