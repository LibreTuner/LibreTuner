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

#include "libretuner.h"
#include "vehicle.h"
#include "sessionscanner.h"
#include "protocols/udsprotocol.h"


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
    std::unique_ptr<PlatformLink> link = LT()->platform_link();
    if (!link) {
        QMessageBox(QMessageBox::Critical, tr("Session Scanner Error"), "Failed to create platform link").exec();
        return;
    }
    
    std::unique_ptr<uds::Protocol> uds = link->uds();
    if (!uds) {
        QMessageBox(QMessageBox::Critical, tr("Session Scanner Error"), "Failed to create UDS interface").exec();
        return;
    }
    
    sessionIds_->clear();
    buttonStart_->setEnabled(false);
    buttonStart_->setText(tr("Scanning"));
    
    std::atomic<bool> stopped{false};
    
    SessionScanner scanner;
    // Initialize callbacks
    scanner.setProgressCallback([](float progress) {
        // Stub
    });
    
    scanner.onSuccess([this](uint8_t session_id) {
        QMetaObject::invokeMethod(sessionIds_, "addItem", Q_ARG(QString, QString::number(session_id)));
    });
    
    
    // Task
    std::packaged_task<void()> task([&]() {
        scanner.scan(std::move(uds), spinMinimum_->value(), spinMaximum_->value());
        stopped = true;
    });
    
    auto future = task.get_future();
    
    std::thread thread([&future, &stopped]() {
        future.wait();
        stopped = true;
    });

    // Handle Qt events while scanning
    while (!stopped) {
        QApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
    
    try {
        future.get();
    } catch (const std::runtime_error &error) {
        QMessageBox(QMessageBox::Critical, tr("Session Scanner Error"), error.what()).exec();
    }
    
    buttonStart_->setEnabled(true);
    buttonStart_->setText(tr("Scan"));
}
