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

#include "datalink/datalink.h"
#include "datalog/datalog.h"
#include "datalog/datalogger.h"
#include "definitions/definition.h"
#include "libretuner.h"
#include "backgroundtask.h"
#include "datalogview.h"

DataLoggerWindow::DataLoggerWindow(QWidget *parent) : QWidget(parent), definition_(LT()->platform()), log_(LT()->platform())
{
    setAttribute( Qt::WA_DeleteOnClose, false );
    setWindowTitle("LibreTuner - Data Logger");
    resize(600, 400);

    QLabel *pidLabel = new QLabel("Available PIDs");
    
    pidList_ = new QListWidget;
    pidList_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    
    auto *datalogView = new DataLogView;
    datalogView->setDatalog(&log_);

    /*logOutput_ = new QTreeWidget;
    logOutput_->setHeaderHidden(true);
    logOutput_->setColumnCount(2);
    logOutput_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    logLayout->addWidget(logOutput_);*/

    buttonLog_ = new QPushButton(tr("Start logging"));
    auto *buttonSave = new QPushButton(tr("Save log"));
    
    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(buttonLog_);
    
    auto *logLayout = new QVBoxLayout;
    logLayout->addWidget(datalogView);
    logLayout->addWidget(buttonLog_);
    
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
    auto start = log_.beginTime();
    
    // Simulate coolant temp
    for (int i = 0; i < 10; ++i) {
        log_.add(0, std::make_pair(start + std::chrono::milliseconds(i * 50), (i / 7000.0) * 30 + 30));
    }
}


void DataLoggerWindow::toggleLogger() {
    if (logger_) {
        logger_->disable();
        return;
    }
    try {
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

        connection_ = log_.connectUpdate([this](const DataLog::Data &data, double value, DataLog::TimePoint time) {
            QMetaObject::invokeMethod(this, [this, data, value]() {
                onLogEntry(data, value);
            });
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
}


void DataLoggerWindow::reset()
{
    pidList_->clear();
    //logOutput_->clear();

    if (!definition_) {
        Logger::info("No definition");
        return;
    }

    for (const definition::Pid &pid : definition_->pids) {
        if (!pid.valid) {
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

    // Adjust minimum width to fit all elements
    pidList_->setMinimumWidth(pidList_->sizeHintForColumn(0) + 8 * pidList_->frameWidth());
}
