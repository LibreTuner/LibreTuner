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
#include "tunemanager.h"
#include "logger.h"

CreateTuneDialog::CreateTuneDialog(const RomMeta *base)
    : ui_(new Ui::CreateTuneDialog) {
  ui_->setupUi(this);

  ui_->comboBase->setItemDelegate(new QStyledItemDelegate());

  for (const RomMeta &rom : RomManager::get()->roms()) {
    ui_->comboBase->addItem(QString::fromStdString(rom.name),
                            QVariant(rom.id));
    if (base != nullptr && rom.id == base->id) {
      ui_->comboBase->setCurrentIndex(ui_->comboBase->count() - 1);
    }
  }
}



CreateTuneDialog::~CreateTuneDialog() { delete ui_; }



void CreateTuneDialog::on_buttonCreate_clicked() {
    int romId = ui_->comboBase->currentData().toInt();
    const RomMeta *rom = RomManager::get()->fromId(romId);
    if (rom == nullptr) {
        Logger::warning("Rom with ID '" + std::to_string(romId) + "' no longer exists");
        return;
    }
    
    try {
        TuneManager::get()->createTune(*rom, ui_->lineName->text().toStdString());
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(QStringLiteral("Error while creating tune: ") + e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Tune creation error");
        msgBox.exec();
    }

    close();
}
