//
// Created by altenius on 12/21/18.
//

#include "datalinkswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeView>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>

#include "database/links.h"
#include "libretuner.h"
#include "adddatalinkdialog.h"

DatalinksWidget::DatalinksWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("LibreTuner - Datalinks"));
    resize(600, 400);
    
    auto *buttonAdd = new QPushButton(tr("Add"));
    auto *buttonRemove = new QPushButton(tr("Remove"));
    
    auto *lineName = new QLineEdit;
    auto *linePort = new QLineEdit;

    auto *linksView = new QTreeView;
    linksView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksView->setModel(&LT()->links());
    
    // Layouts
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRemove);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), lineName);
    formLayout->addRow(tr("Port"), linePort);
    
    auto *linksLayout = new QVBoxLayout;
    linksLayout->addWidget(linksView);
    linksLayout->addLayout(formLayout);

    auto *layout = new QHBoxLayout;
    layout->addLayout(linksLayout);
    layout->addLayout(buttonLayout);
    setLayout(layout);
    
    
    
    connect(buttonAdd, &QPushButton::clicked, []() {
        AddDatalinkDialog dlg;
        dlg.exec();
    });
    
    connect(buttonRemove, &QPushButton::clicked, [linksView, this]() {
        QVariant data = LT()->links().data(linksView->currentIndex(), Qt::UserRole);
        if (!data.canConvert<lt::DataLink*>()) {
            return;
        }
        
        auto *link = data.value<lt::DataLink*>();
        LT()->links().remove(link);
        LT()->saveLinks();
    });
}
