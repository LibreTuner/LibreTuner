//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include <QWidget>
#include <QAbstractItemModel>
#include "database/links.h"

class DatalinksWidget : public QWidget {
public:
    explicit DatalinksWidget(QWidget *parent = nullptr);
};


#endif //LIBRETUNER_INTERFACESDIALOG_H
