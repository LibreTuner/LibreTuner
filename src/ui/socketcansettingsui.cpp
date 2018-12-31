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

#include "socketcansettingsui.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <cassert>

SocketCanSettingsUi::SocketCanSettingsUi(QWidget *parent)
    : SettingsWidget(parent), settings_(std::make_shared<SocketCanSettings>()) {
    QLabel *label = new QLabel("Interface:");
    lineEdit_ = new QLineEdit;
    connect(lineEdit_, &QLineEdit::textEdited, this,
            &SocketCanSettingsUi::on_interfaceChanged);

    layout_->addRow(label, lineEdit_);
}

void SocketCanSettingsUi::on_interfaceChanged(const QString &text) {
    // settings_->setInterface(text.toStdString());
}

void SocketCanSettingsUi::setSettings(const InterfaceSettingsPtr &ptr) {
    assert(ptr);
    assert(ptr->type() == InterfaceType::SocketCan);
    settings_ = std::static_pointer_cast<SocketCanSettings>(ptr);

    lineEdit_->setText(QString::fromStdString(settings_->interface()));
    updateUi();
}

void SocketCanSettingsUi::setName(const std::string &name) {
    settings_->setName(name);
}

std::string SocketCanSettingsUi::name() { return settings_->name(); }

InterfaceSettingsPtr SocketCanSettingsUi::settings() {
    if (!checkName()) {
        return nullptr;
    }
    if (settings_->interface().empty()) {
        displayError("Interface must not be empty");
        return nullptr;
    }
    return settings_;
}
