#ifndef LIBRETUNER_CUSTOMCOMBO_H
#define LIBRETUNER_CUSTOMCOMBO_H

#include <QWidget>

class QComboBox;
class QLineEdit;
class QAbstractItemModel;

class CustomComboProxy;

class CustomCombo : public QWidget {
    Q_OBJECT
public:
    explicit CustomCombo(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

    void setValue(const QString &value);
    QString value();

signals:
    void valueChanged();

private:
    QComboBox *combo_;
    QLineEdit *line_;
    CustomComboProxy *model_;
};

#endif // LIBRETUNER_CUSTOMCOMBO_H
