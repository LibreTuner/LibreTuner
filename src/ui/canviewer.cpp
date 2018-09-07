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

#include "canviewer.h"
#include "canlogview.h"
#include "libretuner.h"
#include "protocols/canlog.h"
#include "ui_canviewer.h"

#include <QAbstractItemModel>

CanViewer::CanViewer(QWidget *parent) : QWidget(parent), ui(new Ui::CanViewer) {
    // QWidget *main = new QWidget;
    // mainLayout()->addWidget(main);
    ui->setupUi(this);

    logModel_ = LibreTuner::get()->canLog();
    ui->logView->setModel(logModel_);
    connect(logModel_, &QAbstractItemModel::rowsInserted, this,
            &CanViewer::rowsInserted);
}

CanViewer::~CanViewer() { delete ui; }

void CanViewer::rowsInserted(const QModelIndex & /*parent*/, int /*first*/,
                             int /*last*/) {
    if (ui->autoScroll->isChecked()) {
        ui->logView->scrollToBottom();
    }
}
