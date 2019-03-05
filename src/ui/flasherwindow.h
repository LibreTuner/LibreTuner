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
#include <QDialog>

#include <memory>
#include <thread>

#include "datalinkslistmodel.h"

namespace flash {
class Flashable;
using FlashablePtr = std::shared_ptr<Flashable>;

class Flasher;
using FlasherPtr = std::shared_ptr<Flasher>;
}

class Tune;
class QComboBox;
class AuthOptionsView;

/**
 * @todo write docs
 */
class FlasherWindow : public QDialog {
    Q_OBJECT
public:
    explicit FlasherWindow(QWidget *parent = nullptr);

    virtual ~FlasherWindow() override;
    
    void setTune(Tune *tune);
    
private:
    void buttonTuneClicked();
    void buttonFlashClicked();
    // Verifies form entries and enables or disables buttonFlash
    void verify();
    
    Tune *selectedTune_;
    
    QPushButton *buttonTune_;
    QPushButton *buttonFlash_;
    QComboBox *comboLink_;
    AuthOptionsView *authOptions_;
    
    DataLinksListModel linksModel_;
};

#endif // FLASHWINDOW_H
