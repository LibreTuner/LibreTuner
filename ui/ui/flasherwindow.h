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

#include <QDialog>
#include <QWidget>

#include <memory>
#include <thread>

#include "database/links.h"

namespace lt
{
class Tune;
using TunePtr = std::shared_ptr<Tune>;
} // namespace lt

class QComboBox;
class AuthOptionsView;
class FileSelectWidget;
class QPushButton;
class QStackedWidget;

/**
 * @todo write docs
 */
class FlasherWindow : public QDialog
{
    Q_OBJECT
public:
    explicit FlasherWindow(QWidget * parent = nullptr);

    virtual ~FlasherWindow() override;

    void setTune(const lt::TunePtr & tune);

private slots:
    void nextClicked();
    void previousClicked();

private:
    void buttonTuneClicked();
    void buttonFlashClicked();
    // Verifies form entries and enables or disables buttonFlash
    void verify();

    QWidget * createSelectPage();
    QWidget * createOptionPage();

    lt::TunePtr selectedTune_;

    QPushButton * buttonFlash_;
    QPushButton * buttonNext_;
    QPushButton * buttonPrevious_;
    QPushButton * buttonAdvanced_;

    QComboBox * comboLink_;
    AuthOptionsView * authOptions_;
    QStackedWidget * stack_;

    LinksListModel linksList_;

    FileSelectWidget * fileSelect_;
};

#endif // FLASHWINDOW_H
