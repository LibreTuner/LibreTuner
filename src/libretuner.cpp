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
#include "ui/addinterfacedialog.h"
#include "ui/flashwindow.h"
#include "ui/styledwindow.h"
#include "ui/setupdialog.h"
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
            flashWindow_ = std::make_unique<FlashWindow>(std::move(flasher), std::move(flash));
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


//DataLinkPtr LibreTuner::getDataLink() {
    // Get the default interface
    /*InterfaceSettingsPtr def = InterfaceManager::get().defaultInterface();
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
        msg.setText("Failed to create datalink from default interface\n" +
                    QString(e.what()));
        msg.setWindowTitle("DataLink Error");
        msg.setIcon(QMessageBox::Critical);
        msg.exec();
        return nullptr;
    }*/
//}



std::unique_ptr<PlatformLink> LibreTuner::getVehicleLink() {
    /*QMessageBox msg(QMessageBox::Information, "Querying vehicle",
                    "Searching for a connected vehicle...");
    msg.show();

    try {
        std::unique_ptr<PlatformLink> link = queryVehicleLink();
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
    }*/

    return nullptr;
}



std::unique_ptr<PlatformLink> LibreTuner::queryVehicleLink() {
    /*DataLinkPtr dl = getDataLink();
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
        window.setWindowTitle("Query platform");
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
    return std::make_unique<PlatformLink>(std::move(v), dl);*/
}



void LibreTuner::load_datalinks() {
#ifdef WITH_J2534
    for (std::unique_ptr<datalink::PassThruLink> &link : datalink::detect_passthru_links()) {
        datalinks_.add_link(std::unique_ptr<datalink::Link>(static_cast<datalink::Link*>(link.release())));
    }
#endif
}



LibreTuner::~LibreTuner() { _global = nullptr; }



void LibreTuner::setup() {
    SetupDialog setup;
    setup.setDefinitionModel(DefinitionManager::get());
    setup.setDatalinksModel(&datalinks_);
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


int Links::rowCount(const QModelIndex &parent) const {
    return static_cast<int>(links_.size());
}


Q_DECLARE_METATYPE(datalink::Link*)

QVariant Links::data(const QModelIndex &index, int role) const {
    if (index.column() < 0 || index.column() > 1 || index.row() < 0 || index.row() >= links_.size()) {
        return QVariant();
    }

    const std::unique_ptr<datalink::Link> &link = links_[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case 0:
                    return QString::fromStdString(link->name());
                case 1:
                    return "UNIMPL";
                default:
                    return QVariant();
            }
        case Qt::UserRole:
            return QVariant::fromValue<datalink::Link*>(link.get());
        default:
            return QVariant();
    }
}



QVariant Links::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
        case 0:
            return "Name";
        case 1:
            return "Type";
        default:
            return QVariant();
    }
}



int Links::columnCount(const QModelIndex &parent) const {
    return 2;
}



QModelIndex Links::index(int row, int column, const QModelIndex &parent) const {
    // No parents
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}



QModelIndex Links::parent(const QModelIndex &child) const {
    // No parents
    return QModelIndex();
}
