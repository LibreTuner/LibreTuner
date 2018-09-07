#include "tunewidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
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

#include <QStyle>

#include "libretuner.h"
#include "tune.h"

#include <cassert>

TuneWidget::TuneWidget(const TuneMeta& tune, QWidget *parent)
    : QFrame(parent), tune_(tune) {
  auto *vlayout = new QVBoxLayout(this);

  QLabel *title = new QLabel(QString::fromStdString(tune.name), this);
  title->setAlignment(Qt::AlignCenter);
  vlayout->addWidget(title);

  QPushButton *buttonEdit = new QPushButton(
      style()->standardIcon(QStyle::SP_DialogOpenButton), "Edit", this);
  vlayout->addWidget(buttonEdit);
  connect(buttonEdit, &QPushButton::clicked, this, &TuneWidget::editClicked);

  QPushButton *buttonDelete = new QPushButton(
      style()->standardIcon(QStyle::SP_TrashIcon), "Delete", this);
  vlayout->addWidget(buttonDelete);

  QPushButton *buttonFlash = new QPushButton(
      style()->standardIcon(QStyle::SP_CommandLink), "Flash", this);
  vlayout->addWidget(buttonFlash);
  connect(buttonFlash, &QPushButton::clicked, this, &TuneWidget::flashClicked);

  setLayout(vlayout);

  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  setLineWidth(0);
}

void TuneWidget::editClicked() { LibreTuner::get()->editTune(tune_); }

void TuneWidget::flashClicked() { LibreTuner::get()->flashTune(tune_); }
