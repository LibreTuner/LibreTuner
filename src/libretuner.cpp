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
#include "logger.h"
#include "timerrunloop.h"
#include "ui/styledwindow.h"

#include "lt/libretuner.h"

#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyledItemDelegate>
#include <QTextStream>

#include <fstream>
#include <future>
#include <memory>

#include "serializeddata.h"
#include "uiutil.h"

static LibreTuner *_global;

namespace fs = std::filesystem;

LibreTuner::LibreTuner(int &argc, char *argv[])
    : QApplication(argc, argv), roms_(definitions_) {
    _global = this;

    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(definitions);
    Q_INIT_RESOURCE(style);
    Q_INIT_RESOURCE(codes);

    setOrganizationDomain("libretuner.org");
    setApplicationName("LibreTuner");

    // Setup LT context
    lt::setLogCallback(
        [](const std::string &message) { Logger::debug(message); });

    // intolib rewrite

    // Setup main path
    rootPath_ = fs::current_path();

    definitions_.setPath(rootPath_ / "definitions");
    roms_.setPath(rootPath_ / "roms");
    links_.setPath(rootPath_ / "links.lts");

    // Load definitions
    catchCritical([this]() { definitions_.load(); },
                  tr("Error while loading definitions"));

    // Load roms
    catchCritical([this]() { roms_.loadRoms(); },
                  tr("Error while loading ROMs"));

    // Load links
    catchCritical([this]() { load_datalinks(); },
                  tr("Error while loading datalinks"));

    // Load DTC descriptions
    dtcDescriptions_.load();

    currentDatalink_ = links_.getFirst();
    currentPlatform_ = definitions_.first();

    setWindowIcon(QIcon(":/icons/libretuner_transparent.png"));

#ifdef WIN32
    {
        QFile f(":qdarkstyle/style.qss");
        if (f.exists()) {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            setStyleSheet(ts.readAll());
        }
    }
#endif

    TimerRunLoop::get().startWorker();
    mainWindow_ = new MainWindow;
    mainWindow_->show();
}

/*
std::shared_ptr<TuneData>
LibreTuner::openTune(const std::shared_ptr<Tune> &tune) {
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
    if (!data) {
        return;
    }


    try {
        Flashable flash = data->flashable();


        if (std::unique_ptr<PlatformLink> link = getVehicleLink()) {
            std::unique_ptr<Flasher> flasher = link->flasher();
            if (!flasher) {
                QMessageBox(QMessageBox::Critical, "Flash failure",
                            "Failed to get a valid flash interface for the
    vehicle " "link. Is a flash mode set in the definition file and " "does the
    datalink support it?") .exec(); return;
            }
            flashWindow_ = std::make_unique<FlasherWindow>(std::move(flasher),
    std::move(flash)); flashWindow_->show();
        }
    } catch (const std::exception &e) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Flash error");
        msgBox.setText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
}*/

LibreTuner *LibreTuner::get() { return _global; }

void LibreTuner::load_datalinks() {
    links_.detect();

    try {
        links_.load();
    } catch (const std::runtime_error &err) {
        QMessageBox::warning(nullptr, tr("Datalink database error"),
                             tr("Failed to load datalink save data: ") +
                                 err.what());
    }
}

LibreTuner::~LibreTuner() { _global = nullptr; }

void LibreTuner::setup() {
    // SetupDialog setup;
    // setup.setDefinitionModel(DefinitionManager::get());
    // DataLinksListModel model;
    // setup.setDatalinksModel(&model);
    // setup.exec();
    // currentDefinition_ = setup.platform();
    // currentDatalink_ = setup.datalink();
}

void LibreTuner::saveLinks() {
    try {
        links_.save();
    } catch (const std::runtime_error &err) {
        QMessageBox::critical(nullptr, tr("Datalink save error"),
                              tr("Failed to save datalink database: ") +
                                  err.what());
    }
}

void LibreTuner::setPlatform(const lt::PlatformPtr &platform) {
    currentPlatform_ = platform;

    if (platform) {
        Logger::debug("Set platform to " + platform->name);
    } else {
        Logger::debug("Unset platform");
    }
}

void LibreTuner::setDatalink(lt::DataLink *link) {
    currentDatalink_ = link;

    if (link != nullptr) {
        Logger::debug("Set datalink to " + link->name());
    } else {
        Logger::debug("Unset datalink");
    }
}

lt::PlatformLink LibreTuner::platformLink() const {
    if (currentDatalink_ == nullptr) {
        throw std::runtime_error("no datalink is selected");
    }
    if (!currentPlatform_) {
        throw std::runtime_error("no platform is selected");
    }

    return lt::PlatformLink(*currentDatalink_, *currentPlatform_);
}

struct TableMeta {
    std::size_t id;
    std::vector<uint8_t> data;
};

struct TuneMeta {
    std::string id;
    std::string name;
    std::string romId;
    std::string modelId;
    std::string platformId;
    std::vector<TableMeta> tables;
};

namespace serialize {
template <typename S> void serialize(S &s, const TableMeta &table) {
    s.serialize(table.id);
    s.serialize(table.data);
}

template <typename S> void serialize(S &s, const TuneMeta &tune) {
    s.serialize(tune.id);
    s.serialize(tune.name);
    s.serialize(tune.romId);
    s.serialize(tune.modelId);
    s.serialize(tune.platformId);
    s.serialize(tune.tables);
}

template <typename D> void deserialize(D &d, TableMeta &table) {
    d.deserialize(table.id);
    d.deserialize(table.data);
}

template <typename D> void deserialize(D &d, TuneMeta &tune) {
    d.deserialize(tune.id);
    d.deserialize(tune.name);
    d.deserialize(tune.romId);
    d.deserialize(tune.modelId);
    d.deserialize(tune.platformId);
    d.deserialize(tune.tables);
}
} // namespace serialize

lt::TunePtr LibreTuner::openTune(const std::filesystem::path &path) const {
    std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open tune at " + path.string());
    }

    std::size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char *>(data.data()), fileSize);
    file.close();

    using InputAdapter = serialize::InputBufferAdapter<std::vector<uint8_t>>;

    serialize::Deserializer<InputAdapter> ds(data);
    TuneMeta meta;
    ds.load(meta);

    const RomMeta *romMeta = roms_.fromId(meta.romId);
    if (romMeta == nullptr) {
        throw std::runtime_error("ROM with id " + meta.romId +
                                 " does not exist");
    }

    lt::RomPtr rom = roms_.openRom(*romMeta);
    lt::TunePtr tune = std::make_shared<lt::Tune>(rom);
    tune->setId(meta.id);
    tune->setName(meta.name);
    for (const TableMeta &table : meta.tables) {
        tune->setTable(table.id, table.data.data(), table.data.size());
    }

    return tune;
}

void LibreTuner::saveTune(const lt::Tune &tune,
                          const std::filesystem::path &path) const {
    TuneMeta meta;
    meta.id = tune.id();
    meta.name = tune.name();
    meta.romId = tune.base()->id();
    meta.modelId = tune.base()->model()->id;
    meta.platformId = tune.base()->model()->platform.id;

    for (std::size_t i = 0; i < tune.tables().size(); ++i) {
        const lt::TablePtr &table = tune.tables()[i];
        if (table) {
            TableMeta tableMeta;
            tableMeta.id = i;
            tableMeta.data =
                table->serialize(tune.base()->model()->platform.endianness);
            meta.tables.emplace_back(std::move(tableMeta));
        }
    }

    std::vector<uint8_t> data;
    using OutputAdapter = serialize::OutputBufferAdapter<std::vector<uint8_t>>;

    serialize::Serializer<OutputAdapter> s(data);

    s.save(meta);

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open tune for writing at " +
                                 path.string());
    }
    file.write(reinterpret_cast<const char *>(data.data()), data.size());
}
