//
// Created by altenius on 12/21/18.
//

#include "datalinkswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeView>
#include <QPushButton>

DatalinksWidget::DatalinksWidget(Links &links, QWidget *parent) : QWidget(parent), links_(links) {
    setWindowTitle(tr("LibreTuner - Datalinks"));
    resize(600, 400);
    auto *layout = new QVBoxLayout;

    auto *linksLayout = new QVBoxLayout;

    auto *buttonNew = new QPushButton(tr("New"));
    auto *buttonDelete = new QPushButton(tr("Delete"));
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(buttonNew);
    buttonLayout->addWidget(buttonDelete);

    auto *linksView = new QTreeView;
    linksLayout->addWidget(linksView);



    linksLayout->addLayout(buttonLayout);

    layout->addLayout(linksLayout);

    setLayout(layout);
}
