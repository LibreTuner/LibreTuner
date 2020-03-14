#include "QuickStartDialog.h"
#include "ui_QuickStartDialog.h"

#include <QFileDialog>
#include <QSettings>

QuickStartDialog::QuickStartDialog(QWidget * parent) : QDialog(parent), ui(new Ui::QuickStartDialog)
{
    ui->setupUi(this);

    // Load history
    QSettings settings;
    settings.beginGroup("Quick Start");
    prevCalibrations_ = settings.value("History").toStringList();

    // Setup history model
    prevCalibrationsModel_.setStringList(prevCalibrations_);
    ui->listCalibrations->setModel(&prevCalibrationsModel_);

    connect(ui->listCalibrations, &QListView::activated, [this](const QModelIndex & index) {
        selectedCalibration_ = prevCalibrationsModel_.data(index).toString();
        accept();
    });
}

QuickStartDialog::~QuickStartDialog() { delete ui; }

void QuickStartDialog::on_buttonOpen_clicked()
{
    QString url = QFileDialog::getOpenFileName(this, tr("Open calibration file"), QString(),
                                               "Calibration (*.rom *.bin);;All files (*)");

    if (url.isEmpty() || prevCalibrations_.contains(url))
        return;

    prevCalibrations_.push_front(url);
    selectedCalibration_ = std::move(url);
    accept();
}
