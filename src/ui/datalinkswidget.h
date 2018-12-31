//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include <QWidget>

class Links;

class DatalinksWidget : public QWidget {
public:
    explicit DatalinksWidget(Links &links, QWidget *parent = nullptr);



private:
    Links &links_;
};


#endif //LIBRETUNER_INTERFACESDIALOG_H
