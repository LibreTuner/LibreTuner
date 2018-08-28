#include "diagnosticswidget.h"

#include "libretuner.h"
#include "diagnosticsinterface.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>
#include <QMessageBox>

DiagnosticsWidget::DiagnosticsWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout();
    QPushButton *buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    layout->addWidget(buttonScan);
    connect(buttonScan, &QPushButton::clicked, [this] {
        std::unique_ptr<VehicleLink> link = LibreTuner::get()->getVehicleLink();
        if (!link) {
            return;
        }
        auto diag = link->diagnostics();
        if (!diag) {
            QMessageBox(QMessageBox::Warning, "No diagnostic interface", "The default datalink does not have a supported diagnostic interface").show();
            return;
        }

        // Clear the scan result prior
        scanResult_.clear();
        diag->scan(scanResult_);
    });

    listCodes_ = new QListView;
    listCodes_->setModel(&scanResult_);
    layout->addWidget(listCodes_);

    setLayout(layout);
}
