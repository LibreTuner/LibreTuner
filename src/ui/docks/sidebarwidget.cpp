#include "sidebarwidget.h"
#include "table.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QFormLayout>


SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
    tableDescription_ = new QPlainTextEdit;
    tableDescription_->setReadOnly(true);
    tableDescription_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    QHBoxLayout *tableInfoTitleLayout = new QHBoxLayout;
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
    
    tableForm->addRow(tr("Name"), tableName_);
    tableForm->addRow(tr("Offset"), tableOffset_);
    tableForm->addRow(tr("Width"), tableWidth_);
    tableForm->addRow(tr("Height"), tableHeight_);
    tableForm->addRow(tr("Range"), tableRange_);
    
    
    tableInfo_->setLayout(tableForm);
    
    // Scroll area
    QVBoxLayout *scrollLayout = new QVBoxLayout;
    scrollLayout->setContentsMargins(0, 5, 0, 5);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->addWidget(tableDescription_);
    scrollLayout->addLayout(tableInfoTitleLayout);
    scrollLayout->addWidget(tableInfo_);
    
    QWidget *scrollWidget = new QWidget;
    scrollWidget->setContentsMargins(0, 0, 0, 0);
    scrollWidget->setLayout(scrollLayout);
    
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    
    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(scrollArea);
    // layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(0, 0, 0, 0);
    
    fillTableInfo(nullptr);
}



void SidebarWidget::fillTableInfo(Table* table)
{
    if (!table) {
        tableName_->setText("N/A");
        tableOffset_->setText("N/A");
        tableWidth_->setText("N/A");
        tableHeight_->setText("N/A");
        tableRange_->setText("N/A");
        tableDescription_->setPlainText("No table selected");
        return;
    }
    
    tableName_->setText(QString::fromStdString(table->name()));
    tableOffset_->setText(QString("0x") + QString::number(table->offset(), 16));
    tableWidth_->setText(QString::number(table->width()));
    tableHeight_->setText(QString::number(table->height()));
    tableRange_->setText(QString("%1 - %2").arg(table->minimum()).arg(table->maximum()));
    tableDescription_->setPlainText(QString::fromStdString(table->description()));
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
