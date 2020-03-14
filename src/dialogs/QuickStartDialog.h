#ifndef QUICKSTART_H
#define QUICKSTART_H

#include <QDialog>
#include <QStringListModel>

namespace Ui
{
class QuickStartDialog;
}

class QuickStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuickStartDialog(QWidget * parent = nullptr);
    ~QuickStartDialog() override;

    const QString & selectedCalibration() const { return selectedCalibration_; };

private slots:
    void on_buttonOpen_clicked();

private:
    Ui::QuickStartDialog * ui;

    QString selectedCalibration_;

    QStringList prevCalibrations_;
    QStringListModel prevCalibrationsModel_;
};

#endif // QUICKSTART_H
