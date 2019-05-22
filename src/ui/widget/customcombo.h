#ifndef LIBRETUNER_CUSTOMCOMBO_H
#define LIBRETUNER_CUSTOMCOMBO_H

#include <QWidget>

class QComboBox;
class QLineEdit;
class QAbstractItemModel;

class CustomComboProxy;

class CustomCombo : public QWidget {
public:
    explicit CustomCombo(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

    QString value();

private:
    QComboBox *combo_;
    QLineEdit *line_;
    CustomComboProxy *model_;
};


#endif //LIBRETUNER_CUSTOMCOMBO_H
