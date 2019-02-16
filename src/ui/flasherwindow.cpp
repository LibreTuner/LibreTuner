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
#include "backgroundtask.h"

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
    buttonTune_->setDefault(true);
    buttonTune_->setAutoDefault(true);
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
    
    // Buttons
    auto *buttonClose = new QPushButton(tr("Close"));
    auto *buttonAdvanced = new QPushButton(tr("Advanced"));
    buttonAdvanced->setCheckable(true);
    
    buttonFlash_ = new QPushButton(tr("Flash"));
    buttonFlash_->setEnabled(false);

    // Form
    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Tune"), buttonTune_);
    form->addRow(tr("Link"), comboLink_);

    authOptions_ = new AuthOptionsView;
    authOptions_->hide();
    
    // Buttons layout
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonFlash_);
    buttonLayout->addWidget(buttonClose);
    buttonLayout->addWidget(buttonAdvanced);
    
    connect(buttonAdvanced, &QAbstractButton::toggled, authOptions_, &QWidget::setVisible);
    connect(buttonClose, &QPushButton::clicked, this, &QWidget::hide);
    connect(buttonFlash_, &QPushButton::clicked, this, &FlasherWindow::buttonFlashClicked);
    
    // Top layout
    auto *topLayout = new QHBoxLayout;
    topLayout->addLayout(form);
    topLayout->addLayout(buttonLayout);
    
    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    
    layout->addLayout(topLayout);
    layout->addWidget(authOptions_);
    
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

        // Create progress dialog
        QProgressDialog progress(tr("Flashing tune..."), tr("Abort"), 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle(tr("LibreTuner - Flash"));
        progress.setValue(0);
        progress.show();

        flasher->setProgressCallback([&](float prog) {
            QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection, Q_ARG(int, prog * 100));
        });
        
        // Create task
        BackgroundTask<bool()> task([&]() {
            return flasher->flash(tuneData->flashable());
        });
        
        bool canceled = false;
        
        connect(&progress, &QProgressDialog::canceled, [&]() {
            // Alert the user of possible bricking
            QMessageBox msgBox;
            msgBox.setText("Are you sure you want to cancel reprogramming");
            msgBox.setInformativeText("Canceling the reprogramming task WILL leave the ECU in an inoperable state.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            
            if (msgBox.exec() == QMessageBox::Yes) {
                flasher->cancel();
                canceled = true;
            }
        });
        
        task();

        if (!canceled) {
            if (!task.future().get()) {
                throw std::runtime_error("Unknown error");
            } else {
                QMessageBox(QMessageBox::Information, "Flash Finished", "Successfully reprogrammed ECU").exec();
            }
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
        authOptions_->setDefaultOptions(selectedTune_->base()->platform()->flashAuthOptions);

    }
    buttonFlash_->setEnabled(selectedTune_ && comboLink_->currentData(Qt::UserRole).isValid());
}




FlasherWindow::~FlasherWindow() {}
