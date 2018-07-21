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

#include "settingswidget.h"
#include "socketcansettingsui.h"
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <cassert>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
  layout_ = new QFormLayout;
  auto *vlayout = new QVBoxLayout;
  vlayout->setMargin(0);
  vlayout->addLayout(layout_);

  labelError_ = new QLabel;
  labelError_->setVisible(false);
  labelError_->setAlignment(Qt::AlignCenter);
  labelError_->setStyleSheet("QLabel { color: red; }");
  vlayout->addWidget(labelError_);

  QLabel *labelName = new QLabel("Name:");
  editName_ = new QLineEdit;
  connect(editName_, &QLineEdit::textChanged, this,
          &SettingsWidget::on_nameChanged);

  layout_->addRow(labelName, editName_);

  setLayout(vlayout);
}

void SettingsWidget::displayError(const QString &text) {
  labelError_->setText(text);
  labelError_->setVisible(true);
}

void SettingsWidget::on_nameChanged(const QString &text) {
  setName(text.toStdString());
}

std::unique_ptr<SettingsWidget> SettingsWidget::create(InterfaceType type) {
  switch (type) {
#ifdef WITH_SOCKETCAN
  case InterfaceType::SocketCan:
    return std::make_unique<SocketCanSettingsUi>();
#endif
#ifdef WITH_J2534
  case InterfaceType::J2534:

#endif
  default:
    return nullptr;
  }
}

std::unique_ptr<SettingsWidget>
SettingsWidget::create(const InterfaceSettingsPtr &settings) {
  assert(settings);
  auto ptr = create(settings->type());
  if (ptr) {
    ptr->setSettings(settings);
  }
  return std::move(ptr);
}

void SettingsWidget::updateUi() {
  editName_->setText(QString::fromStdString(name()));
}

bool SettingsWidget::checkName() {
  if (name().empty()) {
    displayError("Name must not be empty");
    return false;
  }
  return true;
}
