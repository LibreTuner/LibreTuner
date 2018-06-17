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

#ifndef ADDINTERFACEDIALOG_H
#define ADDINTERFACEDIALOG_H

#include <QDialog>
#include <memory>

#include "settingswidget.h"

namespace Ui {
class AddInterfaceDialog;
}

class AddInterfaceDialog : public QDialog {
  Q_OBJECT

public:
  explicit AddInterfaceDialog(QWidget *parent = 0);
  ~AddInterfaceDialog() override;

private slots:
  void on_comboMode_currentIndexChanged(int index);
  void on_buttonCreate_clicked();

private:
  Ui::AddInterfaceDialog *ui;
  std::unique_ptr<SettingsWidget> customSettings_;

  void replaceSettings(std::unique_ptr<SettingsWidget> widget);
};

#endif // ADDINTERFACEDIALOG_H
