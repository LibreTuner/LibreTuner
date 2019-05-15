#ifndef LIBRETUNER_CUSTOMCOMBO_H
#define LIBRETUNER_CUSTOMCOMBO_H

#include <QWidget>

class QComboBox;

class CustomCombo : public QWidget {
public:
    explicit CustomCombo(QWidget *parent = nullptr);

private:
    QComboBox *combo_;
};


#endif //LIBRETUNER_CUSTOMCOMBO_H
