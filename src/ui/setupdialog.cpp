//
// Created by altenius on 12/21/18.
//

#include "setupdialog.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

SetupDialog::SetupDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("LibreTuner - Initial Setup"));
    auto *layout = new QVBoxLayout;

    auto *formLayout = new QFormLayout;
    comboPlatforms_ = new QComboBox;
    comboPlatforms_->setItemDelegate(new QStyledItemDelegate());
    comboPlatforms_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow(tr("Definition"), comboPlatforms_);

    comboDatalinks_ = new QComboBox;
    comboDatalinks_->setItemDelegate(new QStyledItemDelegate());
    comboDatalinks_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formLayout->addRow(tr("Datalink"), comboDatalinks_);

    layout->addLayout(formLayout);

    auto *buttonFinish = new QPushButton(tr("Finish"));
    connect(buttonFinish, &QPushButton::clicked, [this]() { close(); });

    layout->addWidget(buttonFinish);

    setLayout(layout);
}

void SetupDialog::setDefinitionModel(QAbstractItemModel *model) {
    comboPlatforms_->setModel(model);
}

Q_DECLARE_METATYPE(definition::MainPtr);

definition::MainPtr SetupDialog::platform() {
    QVariant var = comboPlatforms_->currentData(Qt::UserRole);
    if (!var.canConvert<definition::MainPtr>()) {
        return nullptr;
    }

    return var.value<definition::MainPtr>();
}

void SetupDialog::setDatalinksModel(QAbstractItemModel *model) {
    comboDatalinks_->setModel(model);
}

Q_DECLARE_METATYPE(datalink::Link *);

datalink::Link *SetupDialog::datalink() {
    QVariant var = comboDatalinks_->currentData(Qt::UserRole);
    if (!var.canConvert<datalink::Link *>()) {
        return nullptr;
    }

    return var.value<datalink::Link *>();
}
