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

#include <lt/project/project.h>

#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QVariant>

CreateTuneDialog::CreateTuneDialog(lt::RomPtr base)
    : ui_(new Ui::CreateTuneDialog)
{
    ui_->setupUi(this);
    ui_->comboBase->setItemDelegate(new QStyledItemDelegate());
    on_comboProject_currentIndexChanged(0);
}

CreateTuneDialog::~CreateTuneDialog() { delete ui_; }

void CreateTuneDialog::on_buttonCreate_clicked()
{
    lt::ProjectPtr project = ui_->comboProject->selectedProject();
    if (!project)
    {
        Logger::info("No project selected");
        return;
    }

    QVariant data = ui_->comboBase->currentData();
    if (!data.canConvert<lt::Calibration::MetaData>())
        return;

    auto md = data.value<lt::Calibration::MetaData>();
    // Load base
    catchWarning(
        [&]() {
            lt::RomPtr base = project->getRom(md.path.filename().string());
            if (!base)
            {
                Logger::warning("Failed to find base ROM, has it changed since "
                                "this menu opened?");
                return;
            }

            project->createTune(base, ui_->lineName->text().toStdString())
                ->save();
            accept();
        },
        tr("Error creating tune"));

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

void CreateTuneDialog::on_comboProject_currentIndexChanged(int index)
{
    ui_->comboBase->clear();
    lt::ProjectPtr project = ui_->comboProject->selectedProject();

    // Populate base combo
    for (const auto & rom : project->queryRoms())
    {
        ui_->comboBase->addItem(QString::fromStdString(rom.name),
                                QVariant::fromValue(rom));
    }
}