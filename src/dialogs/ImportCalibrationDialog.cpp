#include "ImportCalibrationDialog.h"
#include "ui_ImportCalibrationDialog.h"

#include <OverboostApp.h>

#include <QFileInfo>>

#include <utility>

ImportCalibrationDialog::ImportCalibrationDialog(QString path, QWidget * parent)
    : QWizard(parent), ui(new Ui::ImportCalibrationDialog), platformsModel_(&OB()->platforms()), path_(std::move(path))
{
    ui->setupUi(this);

    QFileInfo fi(path_);

    ui->comboPlatform->setModel(&platformsModel_);
    ui->linePath->setText(fi.baseName());
}

ImportCalibrationDialog::~ImportCalibrationDialog() { delete ui; }

const lt::Platform * ImportCalibrationDialog::selectedPlatform() const
{
    return ui->comboPlatform->currentData(Qt::UserRole).value<const lt::Platform *>();
}
