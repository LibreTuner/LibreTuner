#ifndef ADDDATALINKDIALOG_H
#define ADDDATALINKDIALOG_H

#include <QDialog>

class QComboBox;

class DataLinkSettings;

/**
 * @todo write docs
 */
class AddDatalinkDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddDatalinkDialog(QWidget *parent = nullptr);

private slots:
    void addClicked();

    void typeIndexChanged(int index);

private:
    DataLinkSettings *settings_;
    QComboBox *comboType_;
};

#endif // ADDDATALINKDIALOG_H
