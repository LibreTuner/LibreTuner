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

#ifndef TUNEWIDGET_H
#define TUNEWIDGET_H

#include <QFrame>
#include <QWidget>

#include <memory>

class Tune;
typedef std::shared_ptr<Tune> TunePtr;

/**
 * @todo write docs
 */
class TuneWidget : public QFrame {
  Q_OBJECT
public:
  TuneWidget(const TunePtr &tune, QWidget *parent = 0);

private:
  TunePtr tune_;

private slots:
  void editClicked();
  void flashClicked();
};

#endif // TUNEWIDGET_H
