#include "datalinkswidget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QVBoxLayout>
#include <QCheckBox>

#include "serial/device.h"

#include "adddatalinkdialog.h"
#include "database/links.h"
#include "libretuner.h"
#include "models/serialportmodel.h"
#include "uiutil.h"
#include "widget/customcombo.h"
#include "widget/datalinksettings.h"

DatalinksWidget::DatalinksWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("LibreTuner - Datalinks"));
    resize(600, 400);

    auto *buttonAdd = new QPushButton(tr("Add"));
    auto *buttonRemove = new QPushButton(tr("Remove"));

    buttonUpdate_ = new QPushButton(tr("Save"));
    buttonUpdate_->setEnabled(false);
    buttonReset_ = new QPushButton(tr("Reset"));
    buttonReset_->setEnabled(false);

    linksView_ = new QTreeView;
    linksView_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksView_->setModel(&LT()->links());

    settings_ = new DataLinkSettings;
    settings_->setEnabled(false);

    // Layouts
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRemove);

    auto *layoutOptButtons = new QVBoxLayout;
    layoutOptButtons->setAlignment(Qt::AlignTop);
    layoutOptButtons->addWidget(buttonUpdate_);
    layoutOptButtons->addWidget(buttonReset_);

    auto *layoutOpt = new QHBoxLayout;
    layoutOpt->addWidget(settings_);
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

    connect(
        linksView_, &QTreeView::activated,
        [this](const QModelIndex & /*index*/) { linkChanged(currentLink()); });

    connect(buttonUpdate_, &QPushButton::clicked, [this]() {
        lt::DataLink *link = currentLink();
        if (link == nullptr) {
            return;
        }

        settings_->apply(*link);
        LT()->saveLinks();
        setButtonsEnabled(false);
    });

    connect(buttonReset_, &QPushButton::clicked, [this]() {
        linkChanged(currentLink());
        setButtonsEnabled(false);
    });

    connect(settings_, &DataLinkSettings::settingChanged, [this]() {
        setButtonsEnabled(true);
    });
}

void DatalinksWidget::linkChanged(lt::DataLink *link) {
    if (link == nullptr) {
        settings_->setEnabled(false);
        setButtonsEnabled(false);
        return;
    }

    settings_->setEnabled(true);
    settings_->setFlags(link->flags());
    settings_->fill(link);

    setButtonsEnabled(false);
}

lt::DataLink *DatalinksWidget::currentLink() const {
    QVariant data =
        LT()->links().data(linksView_->currentIndex(), Qt::UserRole);
    if (!data.canConvert<lt::DataLink *>()) {
        return nullptr;
    }

    return data.value<lt::DataLink *>();
}

void DatalinksWidget::setButtonsEnabled(bool enabled) {
    buttonUpdate_->setEnabled(enabled);
    buttonReset_->setEnabled(enabled);
}
