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

#include "flasherwindow.h"

#include "definitions/definition.h"
#include "flash/flashable.h"
#include "flash/flasher.h"
#include "logger.h"
#include "libretuner.h"
#include "tunedialog.h"
#include "rom.h"
#include "vehicle.h"
#include "flash/flashable.h"
#include "authoptionsview.h"

#include <cassert>

#include <QMessageBox>
#include <QString>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QProgressDialog>

FlasherWindow::FlasherWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("LibreTuner - Flash"));
    
    // Form entries
    buttonTune_ = new QPushButton(tr("None selected"));
    buttonTune_->setMinimumWidth(300);
    connect(buttonTune_, &QPushButton::clicked, [this]() {
        buttonTuneClicked();
    });
    
    comboLink_ = new QComboBox;
    comboLink_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    comboLink_->setItemDelegate(new QStyledItemDelegate());
    comboLink_->setModel(&LT()->datalinks());
    connect(comboLink_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        verify();
    });
    
    buttonFlash_ = new QPushButton(tr("Flash"));
    buttonFlash_->setEnabled(false);

    // Form
    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Tune"), buttonTune_);
    form->addRow(tr("Link"), comboLink_);

    auto *authOptions = new AuthOptionsView;
    
    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    
    layout->addLayout(form);
    layout->addWidget(authOptions);
    layout->addWidget(buttonFlash_);
    
    setLayout(layout);
}

Q_DECLARE_METATYPE(datalink::Link*)

void FlasherWindow::buttonFlashClicked()
{
    try {
        auto link = comboLink_->currentData(Qt::UserRole).value<datalink::Link*>();
        if (link == nullptr) {
            Logger::debug("Invalid link when pressing button - verification failed somewhere");
            return;
        }
        
        if (selectedTune_ == nullptr) {
            Logger::debug("Invalid tune when pressing button - verification failed somewhere");
            return;
        }
        
        std::shared_ptr<TuneData> tuneData = selectedTune_->data();
        
        auto platform = selectedTune_->base()->platform();
        auto platformLink = std::make_unique<PlatformLink>(platform, *link);
        std::unique_ptr<flash::Flasher> flasher = platformLink->flasher();
        
        if (!flasher) {
            Logger::critical("Failed to create flasher");
            QMessageBox(QMessageBox::Critical, tr("Flash failure"), tr("Failed to create flasher for the selected platform and datalink")).exec();
            return;
        }

        QProgressDialog progress(tr("Flashing tune..."), tr("Abort"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle(tr("LibreTuner - Flash"));
        progress.setValue(0);
        progress.show();


        flasher->setProgressCallback([&](float prog) {
            QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection, Q_ARG(int, prog * 100));
        });

        bool success = false;
        std::atomic<bool> stopped(false);

        std::thread worker([&]() {
            success = flasher->flash(tuneData->flashable());
            stopped = true;
        });

        bool canceled = false;

        while (!stopped) {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents);
            if (progress.wasCanceled()) {
                flasher->cancel();
                canceled = true;
            }
        }
        worker.join();

        if (!success && !canceled) {
            throw std::runtime_error("Unknown error");
        } else {
            QMessageBox(QMessageBox::Information, "Flash Finished", "Successfully reprogrammed ECU").exec();
        }
    } catch (const std::runtime_error &err) {
        QMessageBox(QMessageBox::Critical, "Flash Error", err.what()).exec();
    }
}



void FlasherWindow::setTune(Tune* tune)
{
    selectedTune_ = tune;
    verify();
}



void FlasherWindow::buttonTuneClicked()
{
    TuneDialog dlg;
    dlg.exec();
    
    selectedTune_ = dlg.selectedTune();
    verify();
}



void FlasherWindow::verify()
{
    if (selectedTune_ == nullptr) {
        buttonTune_->setText(tr("None selected"));
    } else {
        buttonTune_->setText(QString::fromStdString(selectedTune_->name()));
    }
    buttonFlash_->setEnabled(selectedTune_ && comboLink_->currentData(Qt::UserRole).isValid());
}




FlasherWindow::~FlasherWindow() {}
