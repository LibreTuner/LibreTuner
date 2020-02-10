#include "vehicleinformationwidget.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <libretuner.h>
#include <logger.h>
#include <uiutil.h>

#include <lt/diagnostics/vehicle_info.h>

VehicleInformationWidget::VehicleInformationWidget(QWidget * parent)
{
    auto * buttonScan = new QPushButton(tr("Scan"));
    auto * buttonClose = new QPushButton(tr("Close"));

    auto * layoutButtons = new QVBoxLayout;
    layoutButtons->addWidget(buttonScan);
    layoutButtons->addWidget(buttonClose);
    layoutButtons->addStretch();

    lineVin_ = new QLineEdit;
    lineVin_->setReadOnly(true);
    lineCalibrationId_ = new QLineEdit;
    lineCalibrationId_->setReadOnly(true);
    lineEcuName_ = new QLineEdit;
    lineEcuName_->setReadOnly(true);

    auto * layoutForm = new QFormLayout;
    layoutForm->addRow(tr("VIN"), lineVin_);
    layoutForm->addRow(tr("Calibration Id"), lineCalibrationId_);
    layoutForm->addRow(tr("ECU Name"), lineEcuName_);

    auto * layout = new QHBoxLayout;
    layout->addLayout(layoutForm);
    layout->addLayout(layoutButtons);

    setLayout(layout);

    connect(buttonScan, &QPushButton::clicked, this,
            &VehicleInformationWidget::scan);
    connect(buttonClose, &QPushButton::clicked, this,
            &VehicleInformationWidget::close);
}

void VehicleInformationWidget::scan()
{
    catchWarning(
        [&]() {
            lt::PlatformLink link = LT()->platformLink();

            lt::network::UdsPtr uds = link.uds();
            if (!uds)
            {
                Logger::warning("Platform or link does not support UDS");
            }

            lt::vehicle_info info =
                lt::request_vehicle_info(*uds, lt::ScanPids::All);
            lineVin_->setText(QString::fromStdString(info.vin));
            lineCalibrationId_->setText(
                QString::fromStdString(info.calibration_id));
            lineEcuName_->setText(QString::fromStdString(info.ecu_name));
        },
        tr("Error querying vehicle information"));
}
