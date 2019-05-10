//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include "database/links.h"
#include <QAbstractItemModel>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QTreeView;

class DatalinksWidget : public QWidget {
public:
    explicit DatalinksWidget(QWidget *parent = nullptr);

private:
    void linkChanged(lt::DataLink *link);
    lt::DataLink *currentLink() const;

    QLineEdit *lineName_;
    QLineEdit *linePort_;

    QPushButton *buttonUpdate_;
    QPushButton *buttonReset_;
    QTreeView *linksView_;
};

#endif // LIBRETUNER_INTERFACESDIALOG_H
