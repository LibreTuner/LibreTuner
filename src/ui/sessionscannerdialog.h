#ifndef SESSIONSCANNERDIALOG_H
#define SESSIONSCANNERDIALOG_H

#include <QDialog>
#include <QStringListModel>

class QListWidget;
class QPushButton;
class QSpinBox;

// Scans sessions
class SessionScannerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SessionScannerDialog(QWidget * parent = nullptr);

signals:

public slots:

private:
    QListWidget * sessionIds_;
    QPushButton * buttonStart_;
    QSpinBox * spinMinimum_;
    QSpinBox * spinMaximum_;

    void scan();
};

#endif // SESSIONSCANNERDIALOG_H
