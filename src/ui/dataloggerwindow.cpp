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

#include "datalink/datalink.h"
#include "datalog/datalog.h"
#include "datalog/datalogger.h"
#include "definitions/definition.h"
#include "libretuner.h"
#include "backgroundtask.h"

DataLoggerWindow::DataLoggerWindow(QWidget *parent) : QWidget(parent), definition_(LT()->platform()), log_(LT()->platform())
{
    setAttribute( Qt::WA_DeleteOnClose, false );
    setWindowTitle("LibreTuner - Data Logger");

    auto *hlayout = new QHBoxLayout;
    setLayout(hlayout);

    auto *pidLayout = new QVBoxLayout;
    hlayout->addLayout(pidLayout);

    QLabel *pidLabel = new QLabel("Available PIDs");
    pidLayout->addWidget(pidLabel);

    pidList_ = new QListWidget;
    pidLayout->addWidget(pidList_);

    auto *logLayout = new QVBoxLayout;
    hlayout->addLayout(logLayout);
    logOutput_ = new QTreeWidget;
    logOutput_->setHeaderHidden(true);
    logLayout->addWidget(logOutput_);

    buttonLog_ = new QPushButton("Start logging");
    logLayout->addWidget(buttonLog_);
    connect(buttonLog_, &QPushButton::clicked, this,
            &DataLoggerWindow::buttonClicked);
    reset();
}

DataLoggerWindow::~DataLoggerWindow() = default;

void DataLoggerWindow::showEvent(QShowEvent *event) {
    Q_UNUSED(event)

}

void DataLoggerWindow::hideEvent(QHideEvent * /*event*/) {
    // TODO: ask to save log
    pidList_->clear();
    logger_.reset();
}

void DataLoggerWindow::buttonClicked() {
    std::unique_ptr<PlatformLink> link = LT()->platform_link();
    if (!link) {
        QMessageBox::critical(this, "Datalog error", "A platform link could not be created");
        return;
    }
    
    logger_ = link->logger(log_);
    
    if (!logger_) {
        QMessageBox::critical(this, "Datalog error", "A datalogger could not be created for the platform link");
        return;
    }
    
    connection_ = log_.connectUpdate([this](const DataLog::Data &data, double value) {
        onLogEntry(data, value);
    });
    
    // Add PIDs
    for (QListWidgetItem *item : pidItems_) {
        if (item->checkState() == Qt::Checked) {
            definition::Pid *pid = definition_->getPid(item->data(Qt::UserRole).toInt());
            if (!pid) {
                // Should never happen...
                Logger::warning("Invalid pid " + std::to_string(item->data(Qt::UserRole).toInt()) + " while adding to logger");
                continue;
            }
            logger_->addPid(pid->id, pid->code, pid->formula);
        }
    }
    
    BackgroundTask<void()> task([&]() {
        logger_->run();
    });
    
    task();
    
    // Catch and exceptions
    try {
        task.future().get();
    } catch (const std::runtime_error &error) {
        QMessageBox::critical(this, "Datalog error", error.what());
    }
}


void DataLoggerWindow::onLogEntry(const DataLog::Data& data, double value)
{
    QTreeWidgetItem *item;
    auto it = outputItems_.find(data.id.id);
    if (it != outputItems_.end()) {
        item = it->second;
    } else {
        item = new QTreeWidgetItem;
        item->setData(0, Qt::DisplayRole, QString::fromStdString(data.id.name));
        outputItems_.insert({data.id.id, item});
    }
    
    item->setData(0, Qt::DisplayRole, QString::number(value));
}


void DataLoggerWindow::reset()
{
    pidList_->clear();
    logOutput_->clear();

    if (!definition_) {
        Logger::info("No definition");
        return;
    }

    for (const definition::Pid &pid : definition_->pids) {
        if (!pid.valid) {
            continue;
        }
        try {
            //log_->addData({pid.name, pid.description, pid.id, DataUnit::None});
            //logger_->addPid(pid.id, pid.code, pid.formula);
        } catch (const std::exception &e) {
            Logger::warning(std::string("Error while adding PID ") + std::to_string(pid.id) + ": " + e.what());
            continue;
        }
        auto *item = new QListWidgetItem;
        item->setText(QString::fromStdString(pid.name));
        item->setData(Qt::UserRole, QVariant::fromValue<uint32_t>(pid.id));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        pidList_->addItem(item);
        pidItems_.emplace_back(item);
    }
}
