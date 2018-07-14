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

#include "createtunedialog.h"
#include "ui_createtunedialog.h"

#include "rommanager.h"
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <tunemanager.h>

Q_DECLARE_METATYPE(RomPtr)

CreateTuneDialog::CreateTuneDialog(RomPtr base)
    : ui_(new Ui::CreateTuneDialog) {
  QWidget *main = new QWidget();
  mainLayout()->addWidget(main);
  ui_->setupUi(main);
  mainLayout()->setSizeConstraint(QLayout::SetFixedSize);

  ui_->comboBase->setItemDelegate(new QStyledItemDelegate());

  for (RomPtr &rom : RomManager::get()->roms()) {
    ui_->comboBase->addItem(QString::fromStdString(rom->name()),
                            QVariant::fromValue(rom));
    if (base != nullptr && rom == base) {
      ui_->comboBase->setCurrentIndex(ui_->comboBase->count() - 1);
    }
  }
}

CreateTuneDialog::~CreateTuneDialog() { delete ui_; }

void CreateTuneDialog::on_buttonCreate_clicked() {
  if (!TuneManager::get()->createTune(
          ui_->comboBase->currentData().value<RomPtr>(),
          ui_->lineName->text().toStdString())) {
    QMessageBox msgBox;
    msgBox.setText("Error while creating tune: " +
                   TuneManager::get()->lastError());
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("Tune creation error");
    msgBox.exec();
  }

  close();
}
