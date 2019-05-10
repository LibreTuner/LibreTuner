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

#include "authoptionsview.h"
#include "backgroundtask.h"
#include "fileselectwidget.h"
#include "libretuner.h"
#include "logger.h"
#include "lt/link/platformlink.h"
#include "uiutil.h"

#include <cassert>

#include <QComboBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QStackedWidget>
#include <QString>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

FlasherWindow::FlasherWindow(QWidget *parent)
    : QDialog(parent), linksList_(LT()->links()) {
    setWindowTitle(tr("LibreTuner - Flash"));
    resize(600, 200);

    // Buttons
    auto *buttonClose = new QPushButton(tr("Close"));
    buttonAdvanced_ = new QPushButton(tr("Advanced"));
    buttonAdvanced_->setCheckable(true);
    buttonAdvanced_->setVisible(false);

    buttonFlash_ = new QPushButton(tr("Flash"));
    buttonFlash_->setEnabled(false);

    // Buttons layout
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonFlash_);
    buttonLayout->addWidget(buttonClose);
    buttonLayout->addWidget(buttonAdvanced_);

    // Page buttons
    buttonNext_ = new QPushButton(tr("Next"));
    buttonPrevious_ = new QPushButton(tr("Previous"));
    buttonPrevious_->setEnabled(false);

    auto *controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(buttonPrevious_);
    controlLayout->addWidget(buttonNext_);

    // Pages
    stack_ = new QStackedWidget;
    stack_->addWidget(createSelectPage());
    stack_->addWidget(createOptionPage());
    stack_->setContentsMargins(0, 0, 0, 0);

    // Top layout
    auto *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(stack_);
    topLayout->addLayout(buttonLayout);

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    // layout->setSizeConstraint(QLayout::SetFixedSize);

    layout->addLayout(topLayout);
    layout->addLayout(controlLayout);

    setLayout(layout);

    connect(buttonNext_, &QPushButton::clicked, this,
            &FlasherWindow::nextClicked);
    connect(buttonPrevious_, &QPushButton::clicked, this,
            &FlasherWindow::previousClicked);
    connect(buttonAdvanced_, &QAbstractButton::toggled, authOptions_,
            &QWidget::setVisible);
    connect(buttonClose, &QPushButton::clicked, this, &QWidget::hide);
    connect(buttonFlash_, &QPushButton::clicked, this,
            &FlasherWindow::buttonFlashClicked);
}

void FlasherWindow::buttonFlashClicked() {
    catchCritical(
        [this]() {
            auto link =
                comboLink_->currentData(Qt::UserRole).value<lt::DataLink *>();
            if (link == nullptr) {
                Logger::debug("Invalid link when pressing button - "
                              "verification failed somewhere");
                return;
            }

            if (!selectedTune_) {
                Logger::debug("Invalid tune when pressing button - "
                              "verification failed somewhere");
                return;
            }

            auto platform = selectedTune_->base()->model()->platform;
            auto platformLink =
                std::make_unique<lt::PlatformLink>(*link, platform);
            lt::FlasherPtr flasher = platformLink->flasher();

            if (!flasher) {
                Logger::critical("Failed to create flasher");
                QMessageBox(QMessageBox::Critical, tr("Flash failure"),
                            tr("Failed to create flasher for the selected "
                               "platform and datalink"))
                    .exec();
                return;
            }

            // Create progress dialog
            QProgressDialog progress(tr("Flashing tune..."), tr("Abort"), 0,
                                     100, this);
            progress.setWindowModality(Qt::WindowModal);
            progress.setWindowTitle(tr("LibreTuner - Flash"));
            progress.setValue(0);
            progress.show();

            flasher->setProgressCallback([&](float prog) {
                QMetaObject::invokeMethod(&progress, "setValue",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, prog * 100));
            });

            // Create task
            BackgroundTask<bool()> task([&]() {
                return flasher->flash(lt::FlashMap::fromTune(*selectedTune_));
            });

            bool canceled = false;

            connect(&progress, &QProgressDialog::canceled, [&]() {
                // Alert the user of possible bricking
                QMessageBox msgBox;
                msgBox.setText("Are you sure you want to cancel reprogramming");
                msgBox.setInformativeText(
                    "Canceling the reprogramming task WILL leave the ECU in an "
                    "inoperable state.");
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
                    QMessageBox(QMessageBox::Information, "Flash Finished",
                                "Successfully reprogrammed ECU")
                        .exec();
                }
            }
        },
        tr("Error while flashing"));
}

void FlasherWindow::setTune(const lt::TunePtr &tune) {
    selectedTune_ = tune;
    if (selectedTune_) {
        stack_->setCurrentIndex(1);
        buttonPrevious_->setEnabled(false);
        buttonNext_->setEnabled(false);
        buttonAdvanced_->setVisible(true);
    }
    verify();
}

void FlasherWindow::nextClicked() {
    // Try to open tune
    catchCritical(
        [this]() {
            selectedTune_ = LT()->openTune(fileSelect_->path().toStdString());

            stack_->setCurrentIndex(1);
            buttonNext_->setEnabled(false);
            buttonPrevious_->setEnabled(true);
            buttonAdvanced_->setVisible(true);

            verify();
        },
        "Error opening tune");
}

void FlasherWindow::previousClicked() {
    stack_->setCurrentIndex(0);
    buttonPrevious_->setEnabled(false);
    buttonNext_->setEnabled(true);
    buttonAdvanced_->setVisible(false);
}

void FlasherWindow::verify() {
    if (selectedTune_ != nullptr) {
        authOptions_->setDefaultOptions(
            selectedTune_->base()->model()->platform.flashAuthOptions);
    }
    buttonFlash_->setEnabled(selectedTune_ &&
                             comboLink_->currentData(Qt::UserRole).isValid());
}

QWidget *FlasherWindow::createSelectPage() {
    fileSelect_ = new FileSelectWidget("Open tune", "Tune (*.ltt)");

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
    layout->addWidget(fileSelect_);

    auto *page = new QWidget;
    page->setLayout(layout);
    page->setContentsMargins(0, 0, 0, 0);
    return page;
}

QWidget *FlasherWindow::createOptionPage() {
    // Form entries
    comboLink_ = new QComboBox;
    comboLink_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    comboLink_->setItemDelegate(new QStyledItemDelegate());
    comboLink_->setModel(&linksList_);
    connect(comboLink_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int) { verify(); });

    // Form
    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Link"), comboLink_);

    authOptions_ = new AuthOptionsView;
    authOptions_->hide();

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop);
    layout->addLayout(form);
    layout->addWidget(authOptions_);

    auto *page = new QWidget;
    page->setLayout(layout);
    page->setContentsMargins(0, 0, 0, 0);
    return page;
}

FlasherWindow::~FlasherWindow() {}
