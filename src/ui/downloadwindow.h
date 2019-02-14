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

#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QDialog>
#include <QWidget>

#include <memory>

class QLineEdit;
class QComboBox;
class LibreTuner;
class PlatformLink;
class AuthOptionsView;

/**
 * Window for downloading firmware from the ECU
 */
class DownloadWindow : public QDialog {
    Q_OBJECT
public:
    explicit DownloadWindow(QWidget *parent = nullptr);
    ~DownloadWindow() override;
    
public slots:
    void download();
    void platformChanged(int index);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
    
private:
    QComboBox *comboPlatform_;
    QLineEdit *lineName_;
    QLineEdit *lineId_;
    AuthOptionsView *authOptions_;

    std::unique_ptr<PlatformLink> get_platform_link();
};

#endif // DOWNLOADWINDOW_H
