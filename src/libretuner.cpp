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
#include "ui/flasherwindow.h"
#include "ui/styledwindow.h"
#include "ui/setupdialog.h"
#include "ui/datalinkslistmodel.h"
#include "vehicle.h"

#ifdef WITH_SOCKETCAN
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#endif

#ifdef WITH_J2534
#include "j2534/j2534.h"
#include "datalink/passthru.h"
#endif

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTextStream>

#include <future>
#include <memory>

static LibreTuner *_global;

LibreTuner::LibreTuner(int &argc, char *argv[]) : QApplication(argc, argv) {
    _global = this;

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(definitions);
    Q_INIT_RESOURCE(style);
    Q_INIT_RESOURCE(codes);

    setOrganizationDomain("libretuner.org");
    setApplicationName("LibreTuner");

    {
        QFile f(":qdarkstyle/style.qss");
        if (f.exists()) {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            setStyleSheet(ts.readAll());
        }
    }

    home_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

#ifdef WITH_SOCKETCAN
    SocketHandler::get()->initialize();
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
        RomStore::get()->load();
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
        RomStore::get()->loadTunes();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText(
            QStringLiteral("Could not load tune metadata from tunes.xml: ") +
            e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("TuneManager error");
        msgBox.exec();
    }
    
    links_.setPath((home_ + "/links.dat").toStdString());

    try {
        load_datalinks();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setText("Could not load interface data from interfaces.xml: " +
                       QString(e.what()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Datalink error");
        msgBox.exec();
    }

    checkHome();

    dtcDescriptions_.load();


    setup();


    mainWindow_ = new MainWindow;
    mainWindow_->setWindowIcon(QIcon(":/icons/LibreTuner.png"));
    mainWindow_->show();



    /*QFile file(":/stylesheet.qss");
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(file.readAll());
        file.close();
    }*/
}



std::shared_ptr<TuneData> LibreTuner::openTune(const std::shared_ptr<Tune> &tune)
{
    std::shared_ptr<TuneData> data;
    try {
        data = tune->data();
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Tune data error");
        msgBox.setText(QStringLiteral("Error opening tune: ") + e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    return data;
}



void LibreTuner::flashTune(const std::shared_ptr<TuneData> &data) {
    /*if (!data) {
        return;
    }


    try {
        Flashable flash = data->flashable();
        
        if (std::unique_ptr<PlatformLink> link = getVehicleLink()) {
            std::unique_ptr<Flasher> flasher = link->flasher();
            if (!flasher) {
                QMessageBox(QMessageBox::Critical, "Flash failure",
                            "Failed to get a valid flash interface for the vehicle "
                            "link. Is a flash mode set in the definition file and "
                            "does the datalink support it?")
                    .exec();
                return;
            }
            flashWindow_ = std::make_unique<FlasherWindow>(std::move(flasher), std::move(flash));
            flashWindow_->show();
        }
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Flash error");
        msgBox.setText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }*/

    
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



void LibreTuner::load_datalinks() {
    links_.detect();
    
    try {
        links_.load();
    } catch (const std::runtime_error &err) {
        QMessageBox::warning(nullptr, tr("Datalink database error"), tr("Failed to load datalink save data: ") + err.what());
    }
}



LibreTuner::~LibreTuner() {
    try {
        links_.save();
    } catch (const std::runtime_error &err) {
        QMessageBox::critical(nullptr, tr("Datalink save error"), tr("Failed to save datalink database: ") + err.what());
    }
    _global = nullptr;
}



void LibreTuner::setup() {
    SetupDialog setup;
    setup.setDefinitionModel(DefinitionManager::get());
    DataLinksListModel model;
    model.setDatabase(&links_);
    setup.setDatalinksModel(&model);
    setup.exec();
    currentDefinition_ = setup.platform();
    currentDatalink_ = setup.datalink();
}

std::unique_ptr<PlatformLink> LibreTuner::platform_link() {
    if (!currentDefinition_ || !currentDatalink_) {
        return nullptr;
    }

    return std::make_unique<PlatformLink>(currentDefinition_, *currentDatalink_);
}



void LibreTuner::setPlatform(const definition::MainPtr &platform) {
    currentDefinition_ = platform;

    if (platform) {
        Logger::debug("Set platform to " + platform->name);
    } else {
        Logger::debug("Unset platform");
    }
}


void LibreTuner::setDatalink(datalink::Link *link) {
    currentDatalink_ = link;

    if (link != nullptr) {
        Logger::debug("Set datalink to " + link->name());
    } else {
        Logger::debug("Unset datalink");
    }
}
