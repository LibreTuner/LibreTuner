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

#ifndef ROMWIDGET_H
#define ROMWIDGET_H

#include <memory>

#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>

class QPushButton;

class Rom;
typedef std::shared_ptr<Rom> RomPtr;

/**
 * Widget for the ROM list
 */
class RomWidget : public QFrame {
  Q_OBJECT
public:
  explicit RomWidget(RomPtr rom, QWidget *parent = 0);

private:
  QLabel *label_;
  QPushButton *deleteButton_;
  QPushButton *tuneButton_;

  RomPtr rom_;

public slots:
  void createTuneClicked();
};

#endif // ROMWIDGET_H
