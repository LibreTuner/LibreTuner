#include "ImportCalibrationDialog.h"
#include "ui_ImportCalibrationDialog.h"

#include <OverboostApp.h>

#include <QFileInfo>
#include <QSettings>

#include <utility>

ImportCalibrationDialog::ImportCalibrationDialog(QString path, QWidget * parent)
    : QWizard(parent), ui(new Ui::ImportCalibrationDialog), platformsModel_(&OB()->platforms()), path_(std::move(path))
{
    ui->setupUi(this);

    QFileInfo fi(path_);

    ui->comboPlatform->setModel(&platformsModel_);
    ui->linePath->setText(fi.baseName());

    // Load selected platform
    QSettings settings;
    if (QString selectedSetting = settings.value("SelectedPlatform").toString(); !selectedSetting.isEmpty())
    {
        std::string selected = selectedSetting.toStdString();
        for (int r = 0; r < platformsModel_.rowCount(QModelIndex()); ++r)
        {
            auto platform = platformsModel_.data(platformsModel_.index(r, 0, QModelIndex()), Qt::UserRole)
                                .value<const lt::Platform *>();
            if (platform->id == selected)
            {
                ui->comboPlatform->setCurrentIndex(r);
                break;
            }
        }
    }
}

ImportCalibrationDialog::~ImportCalibrationDialog() { delete ui; }

const lt::Platform * ImportCalibrationDialog::selectedPlatform() const
{
    return ui->comboPlatform->currentData(Qt::UserRole).value<const lt::Platform *>();
}

void ImportCalibrationDialog::closeEvent(QCloseEvent * event)
{
    if (const auto * platform = ui->comboPlatform->currentData(Qt::UserRole).value<const lt::Platform *>();
        platform != nullptr)
    {
        QSettings settings;
        settings.setValue("SelectedPlatform", QString::fromStdString(platform->id));
    }

    QDialog::closeEvent(event);
}
