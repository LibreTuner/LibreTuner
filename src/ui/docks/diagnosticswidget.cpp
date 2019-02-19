#include "diagnosticswidget.h"

#include "diagnosticsinterface.h"
#include "libretuner.h"
#include "logger.h"
#include "vehicle.h"

#include <QBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

DiagnosticsWidget::DiagnosticsWidget(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout;
    auto *menuLayout = new QHBoxLayout;

    QPushButton *buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    buttonScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    menuLayout->addWidget(buttonScan);

    checkPending_ = new QCheckBox(tr("Scan Pending Codes"));
    checkPending_->setChecked(true);
    menuLayout->addWidget(checkPending_);

    layout->addLayout(menuLayout);

    listCodes_ = new QTableView;
    listCodes_->setModel(&scanResult_);
    listCodes_->horizontalHeader()->setStretchLastSection(true);
    listCodes_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    listCodes_->verticalHeader()->setVisible(false);
    layout->addWidget(listCodes_);

    layout->addWidget(new QLabel(tr("Pending codes")));

    auto *listPending = new QTableView;
    listPending->setModel(&pendingScanResult_);
    listPending->horizontalHeader()->setStretchLastSection(true);
    listPending->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    listPending->verticalHeader()->setVisible(false);
    layout->addWidget(listPending);

    setLayout(layout);

    connect(buttonScan, &QPushButton::clicked, this, &DiagnosticsWidget::scan);
}

void DiagnosticsWidget::scan()
{
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
    pendingScanResult_.clear();
    try {
        diag->scan(scanResult_);
        if (checkPending_->isChecked()) {
            diag->scan(pendingScanResult_);
        }
    } catch (const std::exception &e) {
        Logger::critical("Scan error: " + std::string(e.what()));
    }
}
