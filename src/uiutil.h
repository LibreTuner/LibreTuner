#ifndef UIUTIL_H
#define UIUTIL_H

#include <QMessageBox>

template <typename Func> void catchCritical(Func &&func, const QString &title) {
    try {
        func();
    } catch (const std::exception &err) {
        QMessageBox::critical(nullptr, title, err.what());
    }
}

template <typename Func> void catchWarning(Func &&func, const QString &title) {
    try {
        func();
    } catch (const std::exception &err) {
        QMessageBox::warning(nullptr, title, err.what());
    }
}

#endif