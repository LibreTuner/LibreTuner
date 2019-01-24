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
#include <thread>

#include "flash/flasher.h"
#include "styledwindow.h"

namespace Ui {
class FlashWindow;
}

class Flashable;
typedef std::shared_ptr<Flashable> FlashablePtr;

class Flasher;
using FlasherPtr = std::shared_ptr<Flasher>;

/**
 * @todo write docs
 */
class FlashWindow : public QDialog {
    Q_OBJECT
public:
    FlashWindow(std::unique_ptr<Flasher> &&flasher_,
                Flashable &&flashable);

    virtual ~FlashWindow() override;

    void onCompletion();
    void onError(const std::string &error);
    void onProgress(float percent);

private slots:
    void on_buttonCancel_clicked();
    void on_buttonFlash_clicked();
    void mainError(const QString &error);
    void mainCompletion();

private:
    Ui::FlashWindow *ui;
    Flashable flashable_;
    std::unique_ptr<Flasher> flasher_;
    std::thread worker_;

    void stop();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // FLASHWINDOW_H
