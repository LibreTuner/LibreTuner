#include "diagnosticswidget.h"

#include "diagnosticsinterface.h"
#include "libretuner.h"
#include "logger.h"
#include "vehicle.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>

DiagnosticsWidget::DiagnosticsWidget(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout;
    auto *menuLayout = new QHBoxLayout;

    QPushButton *buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    buttonScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    menuLayout->addWidget(buttonScan);

    //menuLayout->addWidget(new QLabel(tr("Mode:")));

    //auto *buttonUds = new QRadioButton(tr("UDS"));
    //buttonUds->setChecked(true);
    //modeGroup_.addButton(buttonUds);
    //menuLayout->addWidget(buttonUds);

    layout->addLayout(menuLayout);

    listCodes_ = new QTableView;
    listCodes_->setModel(&scanResult_);
    listCodes_->horizontalHeader()->setStretchLastSection(true);
    listCodes_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    listCodes_->verticalHeader()->setVisible(false);
    layout->addWidget(listCodes_);

    setLayout(layout);




    connect(buttonScan, &QPushButton::clicked, [this] {
        std::unique_ptr<PlatformLink> link = LT()->platform_link();
        if (!link) {
            QMessageBox(QMessageBox::Critical, "Diagnostics Error", "Invalid platform or data link. Please run setup again").exec();
            return;
        }
        std::unique_ptr<DiagnosticsInterface> diag;
        try {
            diag = link->diagnostics();
        } catch (const std::exception &e) {
            QMessageBox(QMessageBox::Warning, "No diagnostic interface",
                        "Failed to load diagnostic interface: " +
                        QString(e.what()))
                    .exec();
            return;
        }
        if (!diag) {
            QMessageBox(QMessageBox::Warning, "No diagnostic interface",
                        "The default datalink does not have a supported "
                        "diagnostic interface")
                    .exec();
            return;
        }

        // Clear the scan result prior
        scanResult_.clear();
        try {
            diag->scan(scanResult_);
        } catch (const std::exception &e) {
            Logger::critical("Scan error: " + std::string(e.what()));
        }
    });
}
