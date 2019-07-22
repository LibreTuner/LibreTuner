#ifndef LIBRETUNER_NEWPROJECTDIALOG_H
#define LIBRETUNER_NEWPROJECTDIALOG_H

#include <QDialog>

class QLineEdit;

class NewProjectDialog : public QDialog
{
public:
    explicit NewProjectDialog(QWidget * parent = nullptr);

    QString path() const;

    /* Returns true if the project should be opened instead of created. */
    bool open() const noexcept { return openProject_; }

private:
    QLineEdit * lineName_;
    QLineEdit * linePath_;
    bool openProject_{false};
};

#endif // LIBRETUNER_NEWPROJECTDIALOG_H
