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

    buttonUpdate_ = new QPushButton(tr("Save"));
    buttonUpdate_->setEnabled(false);
    buttonReset_ = new QPushButton(tr("Reset"));
    buttonReset_->setEnabled(false);
    
    lineName_ = new QLineEdit;
    linePort_ = new QLineEdit;
    lineName_->setEnabled(false);
    linePort_->setEnabled(false);

    linksView_ = new QTreeView;
    linksView_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksView_->setModel(&LT()->links());
    
    // Layouts
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRemove);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), lineName_);
    formLayout->addRow(tr("Port"), linePort_);

    auto *layoutOptButtons = new QVBoxLayout;
    layoutOptButtons->addWidget(buttonUpdate_);
    layoutOptButtons->addWidget(buttonReset_);

    auto *layoutOpt = new QHBoxLayout;
    layoutOpt->addLayout(formLayout);
    layoutOpt->addLayout(layoutOptButtons);
    
    auto *linksLayout = new QVBoxLayout;
    linksLayout->addWidget(linksView_);
    linksLayout->addLayout(layoutOpt);

    auto *layout = new QHBoxLayout;
    layout->addLayout(linksLayout);
    layout->addLayout(buttonLayout);
    setLayout(layout);

    connect(buttonAdd, &QPushButton::clicked, []() {
        AddDatalinkDialog dlg;
        dlg.exec();
    });
    
    connect(buttonRemove, &QPushButton::clicked, [this]() {
        auto *link = currentLink();
        if (link == nullptr) {
            return;
        }
        LT()->links().remove(link);
        LT()->saveLinks();
    });
    
    connect(linksView_, &QTreeView::activated, [this](const QModelIndex &index) {
        linkChanged(currentLink());
    });

    connect(buttonUpdate_, &QPushButton::clicked, [this]() {
        lt::DataLink *link = currentLink();
        if (link == nullptr) {
            return;
        }

        link->setName(lineName_->text().toStdString());
        link->setPort(linePort_->text().toStdString());
        LT()->saveLinks();
        buttonUpdate_->setEnabled(false);
        buttonReset_->setEnabled(false);
    });

    connect(buttonReset_, &QPushButton::clicked, [this]() {
        linkChanged(currentLink());
        buttonUpdate_->setEnabled(false);
        buttonReset_->setEnabled(false);
    });

    connect(lineName_, &QLineEdit::textEdited, [this]() {
        buttonUpdate_->setEnabled(true);
        buttonReset_->setEnabled(true);
    });

    connect(linePort_, &QLineEdit::textEdited, [this]() {
        buttonUpdate_->setEnabled(true);
        buttonReset_->setEnabled(true);
    });
}

void DatalinksWidget::linkChanged(lt::DataLink *link)
{
    buttonUpdate_->setEnabled(false);
    buttonReset_->setEnabled(false);

    if (link == nullptr) {
        lineName_->clear();
        linePort_->clear();
        
        lineName_->setEnabled(false);
        linePort_->setEnabled(false);
        return;
    }
    
    lineName_->setText(QString::fromStdString(link->name()));
    linePort_->setText(QString::fromStdString(link->port()));
    lineName_->setEnabled(true);
    linePort_->setEnabled(true);
}

lt::DataLink *DatalinksWidget::currentLink() const {
    QVariant data = LT()->links().data(linksView_->currentIndex(), Qt::UserRole);
    if (!data.canConvert<lt::DataLink*>()) {
        return nullptr;
    }

    return data.value<lt::DataLink*>();
}
