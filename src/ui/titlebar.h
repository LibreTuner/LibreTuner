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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>

class QPushButton;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *window);

    void paintEvent(QPaintEvent *event) override;

    QLabel *title_;

    void setMinimizable(bool minimizable);
    void setMaximizable(bool maximizable);
signals:

public slots:
    void setTitle(const QString &title);
    void setMaximized(bool maximized);

private:
    QWidget *window_;

    QPushButton *restore_;
    QPushButton *maximize_;
    QPushButton *close_;
    QPushButton *minimize_;

    bool maximizable_ = true;
};

#endif // TITLEBAR_H
