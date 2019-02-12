#ifndef SESSIONSCANNERDIALOG_H
#define SESSIONSCANNERDIALOG_H

#include <QDialog>
#include <QStringListModel>

// Scans sessions
class SessionScannerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SessionScannerDialog(QWidget *parent = nullptr);

signals:

public slots:

private:
    QStringListModel sessions_;

    void scan();
};

#endif // SESSIONSCANNERDIALOG_H
