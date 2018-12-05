#include "sidebarwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidget>

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QHBoxLayout *tableInfoTitleLayout = new QHBoxLayout;
    tableInfoButton_ = new QToolButton;
    tableInfoButton_->setArrowType(Qt::DownArrow);
    tableInfoButton_->setAutoRaise(true);
    tableInfoButton_->setCheckable(true);

    connect(tableInfoButton_, &QToolButton::clicked, this, &SidebarWidget::on_treeToolButton_clicked);

    tableInfoTitleLayout->addWidget(tableInfoButton_);
    tableInfoTitleLayout->addWidget(new QLabel(tr("Table Info:")));

    layout->addLayout(tableInfoTitleLayout);

    // Table info tree widget
    tableTreeWidget_ = new QTreeWidget;
    tableTreeWidget_->setFrameShape(QFrame::NoFrame);
    tableTreeWidget_->setHeaderHidden(true);
    tableTreeWidget_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    layout->addWidget(tableTreeWidget_);


    layout->addStretch();
    layout->setContentsMargins(0, 0, 0, 0);
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
