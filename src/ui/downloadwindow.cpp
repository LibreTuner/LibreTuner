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

#include "downloadwindow.h"


#include "definitions/definition.h"
#include "definitions/definitionmanager.h"
#include "logger.h"
#include "protocols/socketcaninterface.h"
#include "vehicle.h"
#include "libretuner.h"
#include "rommanager.h"
#include "download/downloader.h"
#include "backgroundtask.h"

#include "authoptionsview.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QThread>
#include <QProgressDialog>
#include <QComboBox>
#include <QFileDialog>

#include <utility>
#include <thread>
#include <atomic>


DownloadWindow::DownloadWindow(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("LibreTuner - Download"));

    comboPlatform_ = new QComboBox;
    comboPlatform_->setModel(DefinitionManager::get());
    
    lineName_ = new QLineEdit;
    lineId_ = new QLineEdit;

    // Main options
    auto *form = new QFormLayout;
    form->addRow(tr("Platform"), comboPlatform_);
    form->addRow(tr("Name"), lineName_);
    form->addRow(tr("Id"), lineId_);
    
    auto *groupDetails = new QGroupBox(tr("ROM Details"));
    groupDetails->setLayout(form);
    
    // Buttons
    auto *buttonDownload = new QPushButton(tr("Download"));
    buttonDownload->setDefault(true);
    buttonDownload->setAutoDefault(true);
    auto *buttonClose = new QPushButton(tr("Close"));
    auto *buttonAdvanced = new QPushButton(tr("Advanced"));
    buttonAdvanced->setCheckable(true);
    
    // Extension
    authOptions_ = new AuthOptionsView;
    authOptions_->hide();

    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonDownload);
    buttonLayout->addWidget(buttonClose);
    buttonLayout->addWidget(buttonAdvanced);
    
    // Top layout
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(groupDetails);
    topLayout->addLayout(buttonLayout);

    // Main layout
    auto *layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addLayout(topLayout);
    layout->addWidget(authOptions_);
    
    setLayout(layout);
    
    connect(buttonDownload, &QPushButton::clicked, [this]() {
        download();
    });
    connect(buttonClose, &QPushButton::clicked, this, &QWidget::hide);
    connect(buttonAdvanced, &QAbstractButton::toggled, authOptions_, &QWidget::setVisible);
    
    connect(comboPlatform_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DownloadWindow::platformChanged);
    
    platformChanged(comboPlatform_->currentIndex());
}


void DownloadWindow::platformChanged(int index)
{
    QVariant var = comboPlatform_->currentData(Qt::UserRole);
    if (!var.canConvert<definition::MainPtr>()) {
        return;
    }

    auto platform = var.value<definition::MainPtr>();
    if (!platform) {
        return;
    }
    
    authOptions_->setDefaultOptions(platform->downloadAuthOptions);
}



void DownloadWindow::download()
{
    try {
        std::unique_ptr<PlatformLink> pLink = get_platform_link();
        if (!pLink) {
            throw std::runtime_error("Invalid platform or data link");
        }

        // Create progress dialog
        QProgressDialog progress(tr("Downloading ROM..."), tr("Abort"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle(tr("LibreTuner - Download"));
        progress.setValue(0);
        progress.show();


        std::unique_ptr<download::Downloader> downloader = pLink->downloader();
        downloader->setProgressCallback([&](float prog) {
            QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection, Q_ARG(int, prog * 100));
        });
        
        BackgroundTask<bool()> task([&]() -> bool {
            return downloader->download();
        });
        
        bool canceled = false;
        
        connect(&progress, &QProgressDialog::canceled, [&downloader, &canceled]() {
            downloader->cancel();
            canceled = true;
        });

        task();
        
        if (!canceled) {
            bool success = task.future().get();
            if (!success) {
                throw std::runtime_error("Unknown error");
            }
            auto data = downloader->data();
            try {
                RomStore::get()->addRom(lineName_->text().toStdString(),
                                        pLink->definition(), data.first, data.second);
                QMessageBox(QMessageBox::Information, "Download Finished", "ROM downloaded successfully").exec();
            } catch (const std::runtime_error &err) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Download Error");
                msgBox.setText("The ROM was downloaded, but an error occurred while saving. Would you like to save the binary data?");
                msgBox.setInformativeText(err.what());
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);

                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes) {
                    QString fileName = QFileDialog::getSaveFileName(this,
                                                                    tr("Save ROM"), "",
                                                                    tr("Binary (*.bin);;All Files (*)"));
                    if (fileName.isEmpty()) {
                        return;
                    }

                    QFile file(fileName);
                    if (!file.open(QIODevice::WriteOnly)) {
                        QMessageBox::information(this, tr("Unable to open file"),
                                                 file.errorString());
                        return;
                    }
                    file.write(reinterpret_cast<const char*>(data.first), static_cast<qint64>(data.second));
                    file.close();
                }
            }
        }
    } catch (const std::runtime_error &err) {
        QMessageBox(QMessageBox::Critical, "Download Error", err.what()).exec();
    }
}



void DownloadWindow::closeEvent(QCloseEvent* event)
{
}


Q_DECLARE_METATYPE(definition::MainPtr)


std::unique_ptr<PlatformLink> DownloadWindow::get_platform_link() {
    datalink::Link *link = LT()->datalink();
    if (link == nullptr) {
        return nullptr;
    }

    QVariant var = comboPlatform_->currentData(Qt::UserRole);
    if (!var.canConvert<definition::MainPtr>()) {
        return nullptr;
    }

    auto platform = var.value<definition::MainPtr>();
    if (!platform) {
        return nullptr;
    }

    return std::make_unique<PlatformLink>(platform, *link);
}



DownloadWindow::~DownloadWindow()
= default;
