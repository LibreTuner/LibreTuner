#include "sidebarwidget.h"
#include "table.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QScrollArea>
#include <QPlainTextEdit>

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    
    QScrollArea *scrollArea = new QScrollArea;
    QVBoxLayout *scrollLayout = new QVBoxLayout;
    scrollLayout->setContentsMargins(0, 5, 0, 5);
    scrollArea->setLayout(scrollLayout);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
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
    tableTreeWidget_ = new QTreeWidget;
    tableTreeWidget_->setColumnCount(2);
    tableTreeWidget_->setFrameShape(QFrame::NoFrame);
    tableTreeWidget_->setHeaderHidden(true);
    tableTreeWidget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // Build tree widget
    tableName_ = new QTreeWidgetItem(tableTreeWidget_, QStringList{"Name"});
    tableOffset_ = new QTreeWidgetItem(tableTreeWidget_, QStringList("Offset"));
    tableWidth_ = new QTreeWidgetItem(tableTreeWidget_, QStringList("Width"));
    tableHeight_ = new QTreeWidgetItem(tableTreeWidget_, QStringList("Height"));
    tableRange_ = new QTreeWidgetItem(tableTreeWidget_, QStringList("Range"));
    
    scrollLayout->addWidget(tableDescription_);
    scrollLayout->addLayout(tableInfoTitleLayout);
    scrollLayout->addWidget(tableTreeWidget_);
    scrollLayout->addStretch();
    layout->addWidget(scrollArea);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);
}



void SidebarWidget::fillTableInfo(Table* table)
{
    if (!table) {
        tableName_->setText(1, "");
        tableOffset_->setText(1, "");
        tableWidth_->setText(1, "");
        tableHeight_->setText(1, "");
        tableRange_->setText(1, "");
        tableDescription_->setPlainText("");
        return;
    }
    
    tableName_->setText(1, QString::fromStdString(table->name()));
    tableOffset_->setText(1, QString("0x") + QString::number(table->offset(), 16));
    tableWidth_->setText(1, QString::number(table->width()));
    tableHeight_->setText(1, QString::number(table->height()));
    tableRange_->setText(1, QString("Minimum: %1; Maximum: %2").arg(table->minimum()).arg(table->maximum()));
    tableDescription_->setPlainText(QString::fromStdString(table->description()));
}



void SidebarWidget::on_treeToolButton_clicked(bool checked) {
    if (checked) {
        tableInfoButton_->setArrowType(Qt::RightArrow);
        tableTreeWidget_->hide();
    } else {
        tableInfoButton_->setArrowType(Qt::DownArrow);
        tableTreeWidget_->show();
    }
}
