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

#ifndef STYLEDWINDOW_H
#define STYLEDWINDOW_H

#include "titlebar.h"
#include <QDialog>
#include <QMainWindow>
#include <QWidget>
#include <type_traits>

/* A window with custom borders and a title bar on supported platforms */

template <class T> class StyledWidget : public T {
public:
    explicit StyledWidget(QWidget *parent = nullptr);

    QLayout *mainLayout() { return layout_; }

    void setTitle(const QString &title);

    void setResizable(bool resizable);

    bool eventFilter(QObject *object, QEvent *event) override;

#ifdef _WIN32
    bool nativeEvent(const QByteArray &eventType, void *message,
                     long *result) override;
    void changeEvent(QEvent *e) override;
#endif
signals:

public slots:

protected:
#ifdef _WIN32
    TitleBar *titleBar_;
#endif
    QLayout *layout_;

private:
    bool resizable_ = true;
    QLayout *bgLayout_;
};

template <class T> class IntermediateWidget : public T {
public:
    explicit IntermediateWidget(QWidget *parent = nullptr);

    virtual void showEvent(QShowEvent *event) override;

    ~IntermediateWidget() override;

private:
    StyledWidget<QWidget> *parent_;
};

class StyledWindow : public StyledWidget<QWidget> {
    Q_OBJECT
public:
    explicit StyledWindow(QWidget *parent = nullptr);
};

class StyledMainWindow : public StyledWidget<QMainWindow> {
    Q_OBJECT
public:
    explicit StyledMainWindow(QWidget *parent = nullptr);
};

class StyledDialog : public StyledWidget<QDialog> {
    Q_OBJECT
public:
    explicit StyledDialog(QWidget *parent = nullptr);
};



template <class T> void StyledWidget<T>::setTitle(const QString &title) {
#ifdef _WIN32
    titleBar_->setTitle(title);
#else
    T::setWindowTitle(title);
#endif
}

#endif // STYLEDWINDOW_H
