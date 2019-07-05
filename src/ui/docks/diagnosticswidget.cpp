#include "diagnosticswidget.h"

#include "libretuner.h"
#include "logger.h"
#include "uiutil.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

DiagnosticsWidget::DiagnosticsWidget(QWidget * parent) : QWidget(parent)
{
    auto * layout = new QVBoxLayout;
    auto * menuLayout = new QHBoxLayout;

    // Top menu
    // Buttons
    auto * buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    buttonScan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto * buttonClear = new QPushButton(tr("Clear diagnostic trouble codes"));
    buttonClear->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    menuLayout->addWidget(buttonScan);
    menuLayout->addWidget(buttonClear);

    // Options
    checkPending_ = new QCheckBox(tr("Scan Pending Codes"));
    checkPending_->setChecked(true);
    menuLayout->addWidget(checkPending_);

    layout->addLayout(menuLayout);

    // Lists
    listCodes_ = new QTableView;
    listCodes_->setModel(&dtcModel_);
    listCodes_->horizontalHeader()->setStretchLastSection(true);
    listCodes_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    listCodes_->verticalHeader()->setVisible(false);
    layout->addWidget(listCodes_);

    layout->addWidget(new QLabel(tr("Pending codes")));

    auto * listPending = new QTableView;
    listPending->setModel(&pendingDtcModel_);
    listPending->horizontalHeader()->setStretchLastSection(true);
    listPending->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    listPending->verticalHeader()->setVisible(false);
    layout->addWidget(listPending);

    setLayout(layout);

    connect(buttonScan, &QPushButton::clicked, this, &DiagnosticsWidget::scan);
    connect(buttonClear, &QPushButton::clicked, this,
            &DiagnosticsWidget::clear);
}

void DiagnosticsWidget::scan()
{
    catchWarning(
        [this]() {
            lt::PlatformLink link = LT()->platformLink();
            lt::DtcScannerPtr scanner = link.dtcScanner();

            clear();

            dtcModel_.setCodes(scanner->scan());
            if (checkPending_->isChecked())
            {
                pendingDtcModel_.setCodes(scanner->scanPending());
            }
        },
        tr("Diagnostics Scan Error"));
}

void DiagnosticsWidget::clear()
{
    dtcModel_.setCodes(lt::DiagnosticCodes());
    pendingDtcModel_.setCodes(lt::DiagnosticCodes());
}
