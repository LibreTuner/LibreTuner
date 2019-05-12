#ifndef ADDDATALINKDIALOG_H
#define ADDDATALINKDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpinBox;

/**
 * @todo write docs
 */
class AddDatalinkDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddDatalinkDialog(QWidget *parent = nullptr);

private slots:
    void addClicked();

private:
    QComboBox *comboType_;
    QSpinBox *spinBaudrate_;
    QLineEdit *lineName_;
    QLineEdit *linePort_;
};

#endif // ADDDATALINKDIALOG_H
