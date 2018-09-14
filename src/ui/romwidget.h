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

#include <QLabel>
#include <QWidget>

class QPushButton;

struct RomMeta;

/**
 * Widget for the ROM list
 */
class RomWidget : public QWidget {
    Q_OBJECT
public:
    explicit RomWidget(const RomMeta &rom, QWidget *parent = 0);

    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *label_;
    QPushButton *deleteButton_;
    QPushButton *tuneButton_;

    int romId_;

public slots:
    void createTuneClicked();
};

#endif // ROMWIDGET_H
