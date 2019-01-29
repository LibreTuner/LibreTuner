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

#include "flasherwindow.h"

#include "definitions/definition.h"
#include "flash/flashable.h"
#include "flash/flasher.h"
#include "logger.h"

#include <cassert>

#include <QMessageBox>
#include <QString>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>

FlasherWindow::FlasherWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("LibreTuner - Flash"));
    
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    
    QFormLayout *form = new QFormLayout;
    QComboBox *comboTune = new QComboBox;
    form->addRow(tr("Tune"), comboTune);
    QComboBox *comboLink = new QComboBox;
    form->addRow(tr("Link"), comboLink);
    
    comboTune->setItemDelegate(new QStyledItemDelegate());
    comboLink->setItemDelegate(new QStyledItemDelegate());
    
    layout->addLayout(form);
}



FlasherWindow::~FlasherWindow() {}
