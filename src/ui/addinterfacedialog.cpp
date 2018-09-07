/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "addinterfacedialog.h"
#include "ui_addinterfacedialog.h"

#include "interfacemanager.h"
#include "socketcansettingsui.h"

#include <QComboBox>
#include <QMessageBox>
#include <QStyledItemDelegate>

Q_DECLARE_METATYPE(InterfaceType)

AddInterfaceDialog::AddInterfaceDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AddInterfaceDialog) {
    ui->setupUi(this);

    ui->comboMode->setItemDelegate(new QStyledItemDelegate());
    ui->labelError->setVisible(false);

    ui->comboMode->addItem("SocketCAN", QVariant::fromValue<InterfaceType>(
                                            InterfaceType::SocketCan));
    ui->comboMode->addItem(
        "J2534", QVariant::fromValue<InterfaceType>(InterfaceType::J2534));

    on_comboMode_currentIndexChanged(ui->comboMode->currentIndex());
}

AddInterfaceDialog::~AddInterfaceDialog() { delete ui; }

void AddInterfaceDialog::replaceSettings(
    std::unique_ptr<SettingsWidget> widget) {
    if (customSettings_)
        ui->mainLayout->removeWidget(customSettings_.get());
    customSettings_ = std::move(widget);
    if (customSettings_) {
        ui->mainLayout->addWidget(customSettings_.get());
        ui->buttonCreate->setEnabled(true);
    } else {
        ui->buttonCreate->setEnabled(false);
    }
}

void AddInterfaceDialog::on_comboMode_currentIndexChanged(int index) {
    auto iface = SettingsWidget::create(
        ui->comboMode->itemData(index).value<InterfaceType>());
    replaceSettings(std::move(iface));
    if (!customSettings_) {
        ui->labelError->setText(
            "The selected interface is not supported on this platform");
        ui->labelError->setVisible(true);
    } else {
        ui->labelError->setVisible(false);
    }
}

void AddInterfaceDialog::on_buttonCreate_clicked() {
    if (!customSettings_) {
        return;
    }

    auto settings = customSettings_->settings();
    if (settings) {
        InterfaceManager::get().add(settings);
        close();
    }
}
