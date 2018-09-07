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

#include "romwidget.h"
#include "logger.h"
#include "rommanager.h"

#include "createtunedialog.h"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>

RomWidget::RomWidget(const RomMeta &rom, QWidget *parent)
    : QWidget(parent), romId_(rom.id) {
    auto *vlayout = new QVBoxLayout(this);
    auto *hlayout = new QHBoxLayout();
    auto *buttonLayout = new QVBoxLayout();

    label_ = new QLabel(QString::fromStdString(rom.name), this);
    label_->setAlignment(Qt::AlignCenter);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(":/icons/rom-file.png"));
    vlayout->addWidget(label_);
    vlayout->addLayout(hlayout);
    hlayout->addWidget(icon);
    hlayout->addLayout(buttonLayout);

    tuneButton_ = new QPushButton(
        style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Create tune");
    deleteButton_ =
        new QPushButton(style()->standardIcon(QStyle::SP_TrashIcon), "Delete");

    connect(tuneButton_, &QPushButton::clicked, this,
            &RomWidget::createTuneClicked);

    buttonLayout->addWidget(tuneButton_);
    buttonLayout->addWidget(deleteButton_);

    setLayout(vlayout);
    setAutoFillBackground(true);
}

void RomWidget::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

void RomWidget::createTuneClicked() {
    const RomMeta *rom = RomManager::get()->fromId(romId_);
    if (rom == nullptr) {
        Logger::warning("ROM no longer exists");
        // We need not return here because CreateTuneDialog can take a nullptr
        // in its constructor
    }
    CreateTuneDialog dlg(rom);
    dlg.exec();
}
