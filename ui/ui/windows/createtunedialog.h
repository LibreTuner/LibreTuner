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

#ifndef CREATETUNEDIALOG_H
#define CREATETUNEDIALOG_H

#include <QDialog>

#include "lt/rom/rom.h"

#include <memory>

namespace Ui
{
class CreateTuneDialog;
}


namespace lt
{
class Project;
using ProjectPtr = std::shared_ptr<Project>;
}

class ProjectCombo;
/**
 * @todo write docs
 */
class CreateTuneDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateTuneDialog(lt::RomPtr base = nullptr);

    ~CreateTuneDialog() override;

    lt::TunePtr tune() const noexcept { return tune_; }

private:
    Ui::CreateTuneDialog * ui_;
    lt::TunePtr tune_;

private slots:
    void on_buttonCreate_clicked();
    void on_comboProject_currentIndexChanged(int index);
};

#endif // CREATETUNEDIALOG_H
