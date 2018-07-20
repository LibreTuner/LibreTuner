#include "dataloggerwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>
#include <QShowEvent>

#include "libretuner.h"
#include "datalog.h"
#include "datalink.h"
#include "datalogger.h"
#include "definitions/definition.h"

DataLoggerWindow::DataLoggerWindow(QWidget *parent) : QWidget(parent)
{
    setAttribute( Qt::WA_DeleteOnClose, false );
    setWindowTitle("LibreTuner - Data Logger");
    QHBoxLayout *hlayout = new QHBoxLayout;
    setLayout(hlayout);

    QVBoxLayout *pidLayout = new QVBoxLayout;
    hlayout->addLayout(pidLayout);

    QLabel *pidLabel = new QLabel("Available PIDs");
    pidLayout->addWidget(pidLabel);

    pidList_ = new QListWidget;
    pidLayout->addWidget(pidList_);

    QVBoxLayout *logLayout = new QVBoxLayout;
    hlayout->addLayout(logLayout);
    logOutput_ = new QTreeWidget;
    logLayout->addLayout(logLayout);

    buttonLog_ = new QPushButton("Start logging");
    logLayout->addWidget(buttonLog_);
    connect(buttonLog_, &QPushButton::clicked, this, &DataLoggerWindow::buttonClicked);
}

void DataLoggerWindow::showEvent(QShowEvent *event)
{
    pidList_->clear();

    link_ = LibreTuner::get()->getDataLink();
    if (link_) {
        event->accept();
        link_->queryVehicle([this](DataLink::Error error, VehiclePtr vehicle) {
            if (error != DataLink::Error::Success) {
                QMetaObject::invokeMethod(this, [this]() {
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Query failure");
                    msgBox.setText("Failed to query vehicle. Is the datalink device connected?");
                    msgBox.setIcon(QMessageBox::Critical);
                    msgBox.exec();
                    close();
                });
                return;
            }

            vehicle_ = vehicle;
            definition_ = vehicle->definition();
            QMetaObject::invokeMethod(this, "queried", Qt::QueuedConnection);
        });
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("DataLink failure");
        msgBox.setText("No default datalink has been set");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    event->accept();
}

void DataLoggerWindow::hideEvent(QHideEvent *event)
{
    // TODO: ask to save log
    pidList_->clear();
    link_.reset();
    log_.reset();
    logger_.reset();
    vehicle_.reset();
}

void DataLoggerWindow::queried()
{
    if (!definition_) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Unknown vehicle");
        msgBox.setText("The vehicle was queried, but could not be matched to any known definitions");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        close();
        return;
    }

    log_ = std::make_shared<DataLog>();
    logger_ = vehicle_->logger(link_);
    if (!logger_) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Logger error");
        msgBox.setText("Unable to find a suitable logger for the queried vehicle and datalink. Is the 'logmode' tag set in the definition file?");
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        close();
        return;
    }
    // TODO: set error handler
    logger_->setLog(log_);
    // logger_->addPid(0, 5, "X - 40");

    for (const PidDefinition &pid : definition_->pids().pids()) {
        if (!pid.valid) {
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(QString::fromStdString(pid.name));
        item->setData(Qt::UserRole, QVariant::fromValue<uint32_t>(pid.id));
        pidList_->addItem(item);
        logger_->addPid(pid.id, pid.code, pid.formula);
    }
}

void DataLoggerWindow::buttonClicked()
{
    if (!logger_) {
        return;
    }

    if (!logger_->running()) {
        logger_->enable();
        buttonLog_->setText("Stop logging");
    } else {
        logger_->disable();
        buttonLog_->setText("Start logging");
    }
}
