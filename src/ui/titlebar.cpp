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

#include "titlebar.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QApplication>

#ifdef _WIN32
#include <Windows.h>
#endif

TitleBar::TitleBar(QWidget *window) : QWidget(window), window_(window)
{
    auto *layout = new QHBoxLayout;
    layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(30);

    QLabel *icon = new QLabel();
    layout->addWidget(icon);
    icon->setPixmap(QPixmap(":/icons/LibreTuner.png").scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setFixedSize(30, 30);

    title_ = new QLabel();
    layout->addWidget(title_);
    title_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(5, 0, 0, 0);

    minimize_ = new QPushButton();
    layout->addWidget(minimize_);
    minimize_->setIcon(QIcon(":/icons/minimize.svg"));
    minimize_->setIconSize(QSize(12, 12));
    minimize_->setFixedSize(50, 30);
    minimize_->setFlat(true);
    connect(minimize_, &QPushButton::clicked, [this]() {
        window_->setWindowState(Qt::WindowMinimized);
    });

    maximize_ = new QPushButton();
    layout->addWidget(maximize_);
    maximize_->setIcon(QIcon(":/icons/maximize.svg"));
    maximize_->setIconSize(QSize(12, 12));
    maximize_->setFixedSize(50, 30);
    maximize_->setFlat(true);
    connect(maximize_, &QPushButton::clicked, [this]() {
        window_->setWindowState(Qt::WindowMaximized);
    });

    restore_ = new QPushButton();
    layout->addWidget(restore_);
    restore_->setVisible(false);
    restore_->setIcon(QIcon(":/icons/restore.svg"));
    restore_->setIconSize(QSize(12, 12));
    restore_->setFixedSize(50, 30);
    restore_->setFlat(true);
    connect(restore_, &QPushButton::clicked, [this]() {
       window_->setWindowState(Qt::WindowNoState);
    });

    close_ = new QPushButton();
    layout->addWidget(close_);
    close_->setObjectName("close");
    close_->setIcon(QIcon(":/icons/close.svg"));
    close_->setIconSize(QSize(12, 12));
    close_->setFixedSize(50, 30);
    close_->setFlat(true);
    connect(close_, &QPushButton::clicked, [this]() {
        window_->close();
    });

    setLayout(layout);
    setAutoFillBackground(true);
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

void TitleBar::setMinimizable(bool minimizable)
{
    minimize_->setVisible(minimizable);
}

void TitleBar::setMaximizable(bool maximizable)
{
    maximize_->setVisible(maximizable);
    maximizable_ = maximizable;
}

void TitleBar::setTitle(const QString &title)
{
    title_->setText(title);
}

void TitleBar::setMaximized(bool maximized)
{
    if (restore_)
        restore_->setVisible(maximized);
    if (maximize_ && maximizable_)
        maximize_->setVisible(!maximized);

    if (maximized) {
        if (maximize_)
            maximize_->setAttribute(Qt::WA_UnderMouse, false);
    } else {
        if (restore_)
            restore_->setAttribute(Qt::WA_UnderMouse, false);
    }
}

