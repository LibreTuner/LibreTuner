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

#include "libretuner.h"
#include "definitions/definition.h"
#include "logger.h"
#include "rommanager.h"
#include "timerrunloop.h"
#include "tune.h"
#include "tunemanager.h"
#include "ui/addinterfacedialog.h"
#include "ui/flashwindow.h"
#include "ui/styledwindow.h"
#include "ui/tuneeditor.h"
#include "vehicle.h"

#ifdef WITH_SOCKETCAN
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#endif

#ifdef WITH_J2534
#include "j2534/j2534manager.h"
#endif

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyledItemDelegate>

#include <future>
#include <memory>

static LibreTuner *_global;

LibreTuner::LibreTuner(int &argc, char *argv[]) : QApplication(argc, argv) {
    _global = this;

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(definitions);
    Q_INIT_RESOURCE(stylesheet);
    Q_INIT_RESOURCE(codes);

    home_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

#ifdef WITH_SOCKETCAN
    SocketHandler::get()->initialize();
#endif
#ifdef WITH_J2534
    try {
        J2534Manager::get().init();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText("Could not initialize J2534Manager: " +
                       QString(e.what()));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("J2534Manager error");
        msgBox.exec();
    }
#endif
    TimerRunLoop::get().startWorker();

    try {
        DefinitionManager::get()->load();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(
            "Could not load definitions: " +
            QString(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("DefinitionManager error");
        msgBox.exec();
    }

    try {
        RomManager::get()->load();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(
            QStringLiteral("Could not load ROM metadata from roms.xml: ") +
            e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("RomManager error");
        msgBox.exec();
    }

    try {
        TuneManager::get()->load();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(
            QStringLiteral("Could not load tune metadata from tunes.xml: ") +
            e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("TuneManager error");
        msgBox.exec();
    }

    try {
        InterfaceManager::get().load();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText("Could not load interface data from interfaces.xml: " +
                       QString(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("InterfaceManager error");
        msgBox.exec();
    }

    mainWindow_ = std::make_unique<MainWindow>();
    mainWindow_->setWindowIcon(QIcon(":/icons/LibreTuner.png"));
    mainWindow_->show();

    checkHome();

    dtcDescriptions_.load();

    QFile file(":/stylesheet.qss");
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(file.readAll());
        file.close();
    }
}



void LibreTuner::editTune(const TuneMeta &tune) {
    std::shared_ptr<Tune> data;
    try {
        data = std::make_shared<Tune>(tune);
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Tune data error");
        msgBox.setText(QStringLiteral("Error opening tune: ") + e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    tuneEditor_ = std::make_unique<TuneEditor>(data);
    tuneEditor_->show();
}



void LibreTuner::flashTune(const TuneMeta &tune) {
    // TODO: Standard way to open tunes (we repeat the same code in editTune and
    // flashTune... ew)
    std::shared_ptr<Tune> data;
    try {
        data = std::make_shared<Tune>(tune);
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Tune data error");
        msgBox.setText(QStringLiteral("Error opening tune: ") + e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    std::shared_ptr<Flashable> flash;
    try {
        flash = std::make_shared<Flashable>(data);
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Flash error");
        msgBox.setText(
            QStringLiteral("Failed to create flashable from tune: ") +
            e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    if (std::unique_ptr<VehicleLink> link = getVehicleLink()) {
        std::unique_ptr<Flasher> flasher = link->flasher();
        if (!flasher) {
            QMessageBox(QMessageBox::Critical, "Flash failure",
                        "Failed to get a valid flash interface for the vehicle "
                        "link. Is a flash mode set in the definition file and "
                        "does the datalink support it?")
                .exec();
            return;
        }
        flashWindow_ = std::make_unique<FlashWindow>(std::move(flasher), flash);
        flashWindow_->show();
    }
}



LibreTuner *LibreTuner::get() { return _global; }



void LibreTuner::checkHome() {
    QDir home(home_);
    home.mkpath(".");
    home.mkdir("roms");
    home.mkdir("tunes");

    if (!home.exists("definitions")) {
        home.mkdir("definitions");
        // Copy definitions
        QDir dDir(":/definitions");

        for (QFileInfo &info : dDir.entryInfoList(
                 QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort)) {
            QDir realDefDir(home.path() + "/definitions/" + info.fileName() +
                            "/");
            realDefDir.mkpath(".");
            QDir subDir(info.filePath());
            for (QFileInfo &i : subDir.entryInfoList(
                     QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort)) {
                QFile::copy(i.filePath(),
                            realDefDir.path() + "/" + i.fileName());
            }
        }
    }
}



DataLinkPtr LibreTuner::getDataLink() {
    // Get the default interface
    InterfaceSettingsPtr def = InterfaceManager::get().defaultInterface();
    if (!def) {
        // Ask the user to create an interface
        AddInterfaceDialog dlg;
        dlg.exec();
        def = InterfaceManager::get().defaultInterface();
        if (!def) {
            // The user did not create one.
            return nullptr;
        }
    }

    try {
        return DataLink::create(def);
    } catch (const std::exception &e) {
        QMessageBox msg;
        msg.setText("Failed to create datalink from default interface: " +
                    QString(e.what()));
        msg.setWindowTitle("DataLink Error");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
        return nullptr;
    }
}



std::unique_ptr<VehicleLink> LibreTuner::getVehicleLink() {
    QMessageBox msg(QMessageBox::Information, "Querying vehicle",
                    "Searching for a connected vehicle...");
    msg.show();

    try {
        std::unique_ptr<VehicleLink> link = queryVehicleLink();
        msg.hide();

        if (!link) {
            QMessageBox(QMessageBox::Critical, "Query error",
                        "A vehicle could not be queried. Is the device "
                        "connected and ECU active?")
                .exec();
            return nullptr;
        }

        return link;
    } catch (const std::exception &e) {
        QMessageBox(QMessageBox::Critical, "Query error",
                    QString("Error while querying vehicle: ") +
                        QString(e.what()))
            .exec();
    }

    return nullptr;
}



std::unique_ptr<VehicleLink> LibreTuner::queryVehicleLink() {
    DataLinkPtr dl = getDataLink();
    if (!dl) {
        return nullptr;
    }

    Logger::debug("Starting vehicle query");
    Vehicle v;
    try {
        v = dl->queryVehicle();
    } catch (const std::exception &e) {
        Logger::warning("Failed to query vehicle: " + std::string(e.what()));
    }
    if (!v.valid()) {
        // Ask to manually select a vehicle
        StyledDialog window;
        QLabel *label =
            new QLabel("A vehicle could not automatically be queried.\nPlease "
                       "manually select from the list or cancel.");
        label->setAlignment(Qt::AlignCenter);
        window.mainLayout()->addWidget(label);

        QComboBox *combo = new QComboBox;
        int id = 0;
        for (const definition::MainPtr &def : DefinitionManager::get()->definitions()) {
            combo->addItem(QString::fromStdString(def->name), QVariant(id++));
        }
        combo->setItemDelegate(new QStyledItemDelegate());
        window.mainLayout()->addWidget(combo);

        QPushButton *button = new QPushButton("Select");
        window.mainLayout()->addWidget(button);
        connect(button, &QPushButton::clicked, [&v, combo, &window] {
            QVariant data = combo->currentData();
            if (!data.isNull()) {
                const definition::MainPtr &def =
                    DefinitionManager::get()->definitions()[data.toInt()];
                v = Vehicle{def->name, "unknown", def};
            }
            window.close();
        });
        window.exec();
        if (!v.valid()) {
            return nullptr;
        }
    }
    return std::make_unique<VehicleLink>(std::move(v), dl);
}



LibreTuner::~LibreTuner() { _global = nullptr; }

