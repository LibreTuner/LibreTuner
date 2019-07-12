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

#include "backgroundtask.h"
#include "libretuner.h"
#include "logger.h"
#include "uiutil.h"

#include "../database/definitions.h"

#include <lt/project/project.h>

#include "authoptionsview.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>

#include <atomic>
#include <thread>
#include <utility>

DownloadWindow::DownloadWindow(lt::ProjectPtr project, QWidget * parent)
    : QDialog(parent), project_(project)
{
    setWindowTitle(tr("LibreTuner - Download"));

    comboPlatform_ = new QComboBox;
    comboPlatform_->setModel(new PlatformsModel(&LT()->definitions(), this));

    lineName_ = new QLineEdit;

    // Main options
    auto * form = new QFormLayout;
    form->addRow(tr("Platform"), comboPlatform_);
    form->addRow(tr("Name"), lineName_);

    auto * groupDetails = new QGroupBox(tr("ROM Details"));
    groupDetails->setLayout(form);

    // Buttons
    auto * buttonDownload = new QPushButton(tr("Download"));
    buttonDownload->setDefault(true);
    buttonDownload->setAutoDefault(true);
    auto * buttonClose = new QPushButton(tr("Close"));
    auto * buttonAdvanced = new QPushButton(tr("Advanced"));
    buttonAdvanced->setCheckable(true);

    // Extension
    authOptions_ = new AuthOptionsView;
    authOptions_->hide();

    auto * buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonDownload);
    buttonLayout->addWidget(buttonClose);
    buttonLayout->addWidget(buttonAdvanced);

    // Top layout
    auto * topLayout = new QHBoxLayout;
    topLayout->addWidget(groupDetails);
    topLayout->addLayout(buttonLayout);

    // Main layout
    auto * layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addLayout(topLayout);
    layout->addWidget(authOptions_);

    setLayout(layout);

    connect(buttonDownload, &QPushButton::clicked, [this]() { download(); });
    connect(buttonClose, &QPushButton::clicked, this, &QWidget::hide);
    connect(buttonAdvanced, &QAbstractButton::toggled, authOptions_,
            &QWidget::setVisible);

    connect(comboPlatform_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DownloadWindow::platformChanged);

    platformChanged(comboPlatform_->currentIndex());
}

void DownloadWindow::platformChanged(int /*index*/)
{
    QVariant var = comboPlatform_->currentData(Qt::UserRole);
    if (!var.canConvert<lt::PlatformPtr>())
    {
        return;
    }

    const auto & platform = var.value<lt::PlatformPtr>();
    if (!platform)
    {
        return;
    }

    authOptions_->setDefaultOptions(platform->downloadAuthOptions);
}

void DownloadWindow::download()
{
    if (!project_)
    {
        QMessageBox::critical(nullptr, tr("Invalid project"),
                              tr("A project has not been selected."));
        return;
    }
    catchCritical(
        [this]() {
            lt::PlatformLink pLink = getPlatformLink();

            // Create progress dialog
            QProgressDialog progress(tr("Downloading ROM..."), tr("Abort"), 0,
                                     100, this);
            progress.setWindowModality(Qt::WindowModal);
            progress.setWindowTitle(tr("LibreTuner - Download"));
            progress.setValue(0);
            progress.show();

            lt::download::DownloaderPtr downloader = pLink.downloader();
            downloader->setProgressCallback([&](float prog) {
                QMetaObject::invokeMethod(&progress, "setValue",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, prog * 100));
            });

            BackgroundTask<bool()> task(
                [&]() -> bool { return downloader->download(); });

            bool canceled = false;

            connect(&progress, &QProgressDialog::canceled,
                    [&downloader, &canceled]() {
                        downloader->cancel();
                        canceled = true;
                    });

            task();

            if (!canceled)
            {
                bool success = task.future().get();
                if (!success)
                    throw std::runtime_error("Unknown error");
                auto data = downloader->data();
                try
                {
                    lt::ModelPtr model =
                        pLink.platform().identify(data.first, data.second);
                    if (!model)
                        throw std::runtime_error(
                            "Could not identify calibration");

                    lt::RomPtr rom = project_->createRom(
                        lineName_->text().toStdString(), model);
                    rom->setData(std::vector<uint8_t>(
                        data.first, data.first + data.second));
                    rom->save();

                    QMessageBox(QMessageBox::Information, "Download Finished",
                                "ROM downloaded successfully")
                        .exec();
                }
                catch (const std::runtime_error & err)
                {
                    QMessageBox msgBox;
                    msgBox.setWindowTitle("Download Error");
                    msgBox.setText(
                        "The ROM was downloaded, but an error occurred while "
                        "saving. Would you like to save the binary data? "
                        "Please send this to the LibreTuner developers. In the "
                        "future, this will be automatic.");
                    msgBox.setInformativeText(err.what());
                    msgBox.setStandardButtons(QMessageBox::Yes |
                                              QMessageBox::No);
                    msgBox.setDefaultButton(QMessageBox::Yes);

                    int ret = msgBox.exec();
                    if (ret == QMessageBox::Yes)
                    {
                        QString fileName = QFileDialog::getSaveFileName(
                            this, tr("Save ROM"), "",
                            tr("Binary (*.bin);;All Files (*)"));
                        if (fileName.isEmpty())
                            return;

                        QFile file(fileName);
                        if (!file.open(QIODevice::WriteOnly))
                        {
                            QMessageBox::information(this,
                                                     tr("Unable to open file"),
                                                     file.errorString());
                            return;
                        }
                        file.write(reinterpret_cast<const char *>(data.first),
                                   static_cast<qint64>(data.second));
                        file.close();
                    }
                }
            }
        },
        tr("Download error"));
}

void DownloadWindow::closeEvent(QCloseEvent * /*event*/) {}

lt::PlatformLink DownloadWindow::getPlatformLink()
{
    lt::DataLink * link = LT()->datalink();
    if (link == nullptr)
    {
        throw std::runtime_error("no datalink selected");
    }

    QVariant var = comboPlatform_->currentData(Qt::UserRole);
    if (!var.canConvert<const lt::PlatformPtr &>())
    {
        throw std::runtime_error("no platform selected");
    }

    const auto & platform = var.value<lt::PlatformPtr>();
    if (!platform)
    {
        throw std::runtime_error("no platform selected");
    }

    return lt::PlatformLink(*link, *platform);
}

DownloadWindow::~DownloadWindow() = default;
