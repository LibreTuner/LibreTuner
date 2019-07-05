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

#include "libretuner.h"
#include "logger.h"
#include "uiutil.h"

#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QVariant>

CreateTuneDialog::CreateTuneDialog(lt::RomPtr base)
    : ui_(new Ui::CreateTuneDialog)
{
    ui_->setupUi(this);

    ui_->comboBase->setItemDelegate(new QStyledItemDelegate());

    /*
    for (const RomMeta &meta : LT()->roms()) {
        ui_->comboBase->addItem(QString::fromStdString(meta.name),
                                QString::fromStdString(meta.id));
        if (base != nullptr && &meta == base) {
            ui_->comboBase->setCurrentIndex(ui_->comboBase->count() - 1);
        }
    }*/
    // TODO: Search ROM directory
}

CreateTuneDialog::~CreateTuneDialog() { delete ui_; }

void CreateTuneDialog::on_buttonCreate_clicked()
{
    std::string romId = ui_->comboBase->currentData().toString().toStdString();
    /*RomMeta *meta = LT()->roms().fromId(romId);
    if (meta == nullptr) {
        Logger::warning("Rom with ID '" + romId + "' no longer exists");
        return;
    }

    catchWarning(
        [this, meta]() {
            lt::RomPtr rom = LT()->roms().openRom(*meta);
            tune_ = std::make_shared<lt::Tune>(rom);
            tune_->setId(ui_->lineName->text().toStdString());
            tune_->setName(ui_->lineName->text().toStdString());
        },
        tr("Error creating tune"));

    close();*/
}
