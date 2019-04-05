/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dataloggerwindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTimer>

#include "lt/link/datalink.h"
#include "lt/datalog/datalogger.h"
#include "lt/definition/platform.h"
#include "libretuner.h"
#include "backgroundtask.h"
#include "widget/datalogview.h"
#include "widget/datalogliveview.h"

DataLoggerWindow::DataLoggerWindow(QWidget *parent) : QWidget(parent), log_(std::make_shared<lt::DataLog>())
{
    setAttribute( Qt::WA_DeleteOnClose, false );
    setWindowTitle("LibreTuner - Data Logger");
    resize(600, 400);

    QLabel *pidLabel = new QLabel("Available PIDs");
    
    pidList_ = new QListWidget;
    pidList_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    auto *datalogView = new DataLogView;
    datalogView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    datalogView->setDataLog(log_);
    
    auto *dataLogLiveView = new DataLogLiveView;
    dataLogLiveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    dataLogLiveView->setDataLog(log_);

    /*logOutput_ = new QTreeWidget;
    logOutput_->setHeaderHidden(true);
    logOutput_->setColumnCount(2);
    logOutput_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    logLayout->addWidget(logOutput_);*/

    buttonLog_ = new QPushButton(tr("Start logging"));
    auto *buttonSave = new QPushButton(tr("Save log"));
    
    auto *buttonSimulate = new QPushButton(tr("Simulate"));
    
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(buttonLog_);
    
    auto *logLayout = new QVBoxLayout;
    logLayout->addWidget(datalogView);
    logLayout->addWidget(dataLogLiveView);
    logLayout->addWidget(buttonLog_);
    logLayout->addWidget(buttonSimulate);
    
    // PIDs layout
    auto *pidLayout = new QVBoxLayout;
    pidLayout->addWidget(pidLabel);
    pidLayout->addWidget(pidList_);
    
    // Main layout
    auto *hlayout = new QHBoxLayout;
    hlayout->addLayout(pidLayout);
    hlayout->addLayout(logLayout);
    
    setLayout(hlayout);
    
    // Signals
    connect(buttonLog_, &QPushButton::clicked, this,
            &DataLoggerWindow::toggleLogger);
    connect(buttonSave, &QPushButton::clicked, this,
            &DataLoggerWindow::saveLog);
    connect(buttonSimulate, &QPushButton::clicked, [this]() {
        simulate();
    });
    reset();
}

DataLoggerWindow::~DataLoggerWindow() = default;

void DataLoggerWindow::showEvent(QShowEvent *event) {
    Q_UNUSED(event)
}

void DataLoggerWindow::hideEvent(QHideEvent * /*event*/) {
    // TODO: ask to save log
    pidList_->clear();
    if (logger_) {
        logger_->disable();
    }
}


void DataLoggerWindow::saveLog()
{
}


void DataLoggerWindow::simulate()
{
    lt::Pid pid;
    pid.name = "Test";
    pid.code = 0;
    log_->addPid(pid);
    
    /*QTimer timer(this);
    timer.setInterval(1);
    connect(&timer, &QTimer::timeout, [this, &pid]() {
        static std::size_t offset = 0;
        log_->add(pid, lt::PidLogEntry{6, offset * 100});
        offset++;
    });
    timer.start();*/

    // Simulate coolant temp
    for (int i = 0; i < 10; ++i) {
        log_->add(pid, lt::PidLogEntry{i * 50, (i / 7000.0) * 30 + 30});
    }
}


void DataLoggerWindow::toggleLogger() {
    if (logger_) {
        logger_->disable();
        return;
    }
    try {
        lt::PlatformLink link = LT()->platformLink();
        const lt::Platform &platform = link.platform();
        logger_ = link.datalogger(*log_);

        // Add PIDs
        for (QListWidgetItem *item : pidItems_) {
            if (item->checkState() == Qt::Checked) {
                const lt::Pid *pid = platform.getPid(item->data(Qt::UserRole).toInt());
                if (pid == nullptr) {
                    // Should never happen...
                    Logger::warning("Invalid pid " + std::to_string(item->data(Qt::UserRole).toInt()) + " while adding to logger");
                    continue;
                }
                logger_->addPid(*pid);
            }
        }

        BackgroundTask<void()> task([&]() {
            logger_->run();
        });

        buttonLog_->setText(tr("Stop logging"));
        task();

        // Catch any exceptions
        task.future().get();

        logger_.reset();
        buttonLog_->setText(tr("Start logging"));
    } catch (const std::runtime_error &error) {
        QMessageBox::critical(this, "Datalog error", error.what());
    }
}

/*
void DataLoggerWindow::onLogEntry(const DataLog::Data& data, double value)
{
    QTreeWidgetItem *item;
    auto it = outputItems_.find(data.id.id);
    if (it != outputItems_.end()) {
        item = it->second;
    } else {
        item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(data.id.name));
        item->setText(1, "...");
        logOutput_->addTopLevelItem(item);
        outputItems_.insert({data.id.id, item});
    }
    
    item->setData(1, Qt::DisplayRole, value);
}*/


void DataLoggerWindow::reset()
{
    pidList_->clear();
    //logOutput_->clear();
    
    const lt::PlatformPtr &platform = LT()->platform();

    if (!platform) {
        Logger::info("No platform selected");
        return;
    }

    for (const lt::Pid &pid : platform->pids) {
        auto *item = new QListWidgetItem;
        item->setText(QString::fromStdString(pid.name));
        item->setData(Qt::UserRole, QVariant::fromValue<uint32_t>(pid.code));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        pidList_->addItem(item);
        pidItems_.emplace_back(item);
    }

    // Adjust minimum width to fit all elements
    pidList_->setMinimumWidth(pidList_->sizeHintForColumn(0) + 8 * pidList_->frameWidth());
}
