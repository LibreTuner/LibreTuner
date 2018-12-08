#include "sidebarwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidget>
#include <QScrollArea>

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    
    QScrollArea *scrollArea = new QScrollArea;
    QVBoxLayout *scrollLayout = new QVBoxLayout;
    scrollLayout->setContentsMargins(0, 5, 0, 5);
    scrollArea->setLayout(scrollLayout);

    QHBoxLayout *tableInfoTitleLayout = new QHBoxLayout;
    tableInfoButton_ = new QToolButton;
    tableInfoButton_->setArrowType(Qt::DownArrow);
    tableInfoButton_->setAutoRaise(true);
    tableInfoButton_->setCheckable(true);

    connect(tableInfoButton_, &QToolButton::clicked, this, &SidebarWidget::on_treeToolButton_clicked);

    tableInfoTitleLayout->addWidget(tableInfoButton_);
    tableInfoTitleLayout->addWidget(new QLabel(tr("Table Info:")));

    scrollLayout->addLayout(tableInfoTitleLayout);

    // Table info tree widget
    tableTreeWidget_ = new QTreeWidget;
    tableTreeWidget_->setColumnCount(2);
    tableTreeWidget_->setFrameShape(QFrame::NoFrame);
    tableTreeWidget_->setHeaderHidden(true);
    tableTreeWidget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // Build tree widget
    tableName_ = new QTreeWidgetItem(tableTreeWidget_, QStringList{"Name"});
    tableOffset_ = new QTreeWidgetItem(tableTreeWidget_, QStringList("Offset"));
    
    scrollLayout->addWidget(tableTreeWidget_);
    scrollLayout->addStretch();
    layout->addWidget(scrollArea);
    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);
}



void SidebarWidget::setTableName(const QString& name)
{
    tableName_->setText(1, name);
}



void SidebarWidget::setTableOffset(std::size_t offset)
{
    tableOffset_->setText(1, QString("0x") + QString::number(offset, 16));
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
