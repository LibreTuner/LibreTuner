#ifndef LIBRETUNER_NEWPROJECTDIALOG_H
#define LIBRETUNER_NEWPROJECTDIALOG_H

#include <QDialog>

class QLineEdit;

class NewProjectDialog : public QDialog
{
public:
    explicit NewProjectDialog(QWidget * parent = nullptr);

    QString path() const;

private:
    QLineEdit * lineName_;
    QLineEdit * linePath_;
};

#endif // LIBRETUNER_NEWPROJECTDIALOG_H
