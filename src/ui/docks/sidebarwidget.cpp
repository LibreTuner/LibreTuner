#include "sidebarwidget.h"

#include "lt/definition/model.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QFormLayout>


SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
    tableDescription_ = new QLabel;
    tableDescription_->setWordWrap(true);
    tableDescription_->setAlignment(Qt::AlignCenter);

    auto *tableInfoTitleLayout = new QHBoxLayout;
    tableInfoButton_ = new QToolButton;
    tableInfoButton_->setArrowType(Qt::DownArrow);
    tableInfoButton_->setAutoRaise(true);
    tableInfoButton_->setCheckable(true);

    connect(tableInfoButton_, &QToolButton::clicked, this, &SidebarWidget::on_treeToolButton_clicked);

    tableInfoTitleLayout->addWidget(tableInfoButton_);
    tableInfoTitleLayout->addWidget(new QLabel(tr("Table Info:")));

    // Table info tree widget
    tableInfo_ = new QWidget;
    tableInfo_->setContentsMargins(0, 0, 0, 0);
    auto *tableForm = new QFormLayout;
    tableForm->setSizeConstraint(QFormLayout::SetMaximumSize);
    
    // Build tree widget
    tableName_ = new QLineEdit;
    tableName_->setReadOnly(true);
    tableOffset_ = new QLineEdit;
    tableOffset_->setReadOnly(true);
    tableWidth_ = new QLineEdit;
    tableWidth_->setReadOnly(true);
    tableHeight_ = new QLineEdit;
    tableHeight_->setReadOnly(true);
    tableRange_ = new QLineEdit;
    tableRange_->setReadOnly(true);
    tableUnit_ = new QLineEdit;
    tableUnit_->setReadOnly(true);
    tableDataType_ = new QLineEdit;
    tableDataType_->setReadOnly(true);
    
    tableForm->addRow(tr("Name"), tableName_);
    tableForm->addRow(tr("Offset"), tableOffset_);
    tableForm->addRow(tr("Width"), tableWidth_);
    tableForm->addRow(tr("Height"), tableHeight_);
    tableForm->addRow(tr("Range"), tableRange_);
    tableForm->addRow(tr("Unit"), tableUnit_);
    tableForm->addRow(tr("Data type"), tableDataType_);
    
    
    tableInfo_->setLayout(tableForm);
    
    // Scroll area
    auto *scrollLayout = new QVBoxLayout;
    scrollLayout->setContentsMargins(5, 0, 5, 0);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->addWidget(tableDescription_);
    scrollLayout->addLayout(tableInfoTitleLayout);
    scrollLayout->addWidget(tableInfo_);
    
    auto *scrollWidget = new QWidget;
    scrollWidget->setContentsMargins(0, 0, 0, 0);
    scrollWidget->setLayout(scrollLayout);
    
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    
    // Main layout
    auto *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(scrollArea);
    // layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    
    fillTableInfo(nullptr);
}



void SidebarWidget::fillTableInfo(const lt::ModelTable* mod)
{
    if (mod == nullptr) {
        tableName_->setText("N/A");
        tableOffset_->setText("N/A");
        tableWidth_->setText("N/A");
        tableHeight_->setText("N/A");
        tableRange_->setText("N/A");
        tableDescription_->setText("No table selected");
        tableUnit_->setText("N/A");
        tableDataType_->setText("N/A");
        return;
    }

    const lt::TableDefinition *table = mod->table;
    
    tableName_->setText(QString::fromStdString(table->name));
    tableOffset_->setText(QString("0x") + QString::number(mod->offset, 16));
    tableWidth_->setText(QString::number(table->width));
    tableHeight_->setText(QString::number(table->height));
    tableRange_->setText(QString("%1 - %2").arg(table->minimum).arg(table->maximum));
    tableDescription_->setText(QString::fromStdString(table->description));
}



void SidebarWidget::on_treeToolButton_clicked(bool checked) {
    if (checked) {
        tableInfoButton_->setArrowType(Qt::RightArrow);
        tableInfo_->hide();
    } else {
        tableInfoButton_->setArrowType(Qt::DownArrow);
        tableInfo_->show();
    }
}
