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

#ifndef FLASHWINDOW_H
#define FLASHWINDOW_H

#include <QWidget>

#include <memory>

#include "flasher.h"
#include "styledwindow.h"

namespace Ui {
class FlashWindow;
}

class Flashable;
typedef std::shared_ptr<Flashable> FlashablePtr;

class Flasher;
typedef std::shared_ptr<Flasher> FlasherPtr;

/**
 * @todo write docs
 */
class FlashWindow : public QDialog, public Flasher::Callbacks {
  Q_OBJECT
public:
  FlashWindow(const FlashablePtr &flashable);

  void onCompletion() override;
  void onError(const std::string &error) override;
  void onProgress(double percent) override;

private slots:
  void on_buttonCancel_clicked();
  void on_buttonFlash_clicked();
  void mainError(const QString &error);
  void mainCompletion();

private:
  Ui::FlashWindow *ui;
  FlashablePtr flashable_;
  FlasherPtr flasher_;
};

#endif // FLASHWINDOW_H
