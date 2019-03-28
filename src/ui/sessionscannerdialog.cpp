#include "sessionscannerdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>
#include <QSpinBox>
#include <QLabel>

#include <thread>
#include <atomic>
#include <future>
#include <sstream>

#include "libretuner.h"
#include "lt/session/sessionscanner.h"
#include "backgroundtask.h"

#include "uiutil.h"


SessionScannerDialog::SessionScannerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("LibreTuner - Session Scanner"));

    sessionIds_ = new QListWidget;
    buttonStart_ = new QPushButton(tr("Scan"));
    
    spinMinimum_ = new QSpinBox;
    spinMinimum_->setRange(0, 0xFF);
    spinMinimum_->setDisplayIntegerBase(16);
    spinMinimum_->setPrefix("0x");
    
    spinMaximum_ = new QSpinBox;
    spinMaximum_->setRange(0, 0xFF);
    spinMaximum_->setDisplayIntegerBase(16);
    spinMaximum_->setPrefix("0x");
    spinMaximum_->setValue(0xFF);

    connect(buttonStart_, &QPushButton::clicked, [this]() {
        scan();
    });
    
    auto *layout = new QVBoxLayout;
    
    auto *spinLayout = new QHBoxLayout;
    spinLayout->addWidget(new QLabel(tr("Minimum:")));
    spinLayout->addWidget(spinMinimum_);
    spinLayout->addWidget(new QLabel(tr("Maximum:")));
    spinLayout->addWidget(spinMaximum_);

    layout->addWidget(sessionIds_);
    layout->addLayout(spinLayout);
    layout->addWidget(buttonStart_);

    setLayout(layout);
}



void SessionScannerDialog::scan()
{
    catchCritical([this]() {
        lt::PlatformLink link = LT()->platformLink();
        lt::network::UdsPtr uds = link.uds();

        sessionIds_->clear();
        buttonStart_->setEnabled(false);
        buttonStart_->setText(tr("Scanning"));
        lt::SessionScanner scanner;
        // Initialize callbacks
        scanner.setProgressCallback([](float progress) {
            // Stub
        });

        scanner.onSuccess([this](uint8_t session_id) {
            std::stringstream ss;
            ss << "[SessionScanner] Detected 0x" << std::hex << static_cast<uint32_t>(session_id);
            Logger::debug(ss.str());
            QMetaObject::invokeMethod(sessionIds_, [this, session_id]() {
                sessionIds_->addItem(QString("0x%1").arg(session_id, 0, 16));
            });
        });

        BackgroundTask<void()> task([&]() {
            scanner.scan(*uds, static_cast<uint8_t>(spinMinimum_->value()), static_cast<uint8_t>(spinMaximum_->value()));
        });

        task();
        task.future().get();
    }, tr("Session Scanner Error"));

    buttonStart_->setEnabled(true);
    buttonStart_->setText(tr("Scan"));
}
