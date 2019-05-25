#include "roms.h"

#include "definitions.h"
#include "lt/rom/rom.h"
#include "serializeddata.h"

#include "logger.h"

#include <fstream>

namespace fs = std::filesystem;

struct SerializedRom {
    std::string id;
    std::string name;
    std::string platformId;
    std::string modelId;
};

namespace serialize {
template <typename S> void serialize(S &s, const SerializedRom &meta) {
    s.serialize(meta.id);
    s.serialize(meta.name);
    s.serialize(meta.platformId);
    s.serialize(meta.modelId);
}

// Actually serializes a SerializedRom
template <typename S> void serialize(S &s, const RomMeta &meta) {
    s.serialize(meta.id);
    s.serialize(meta.name);
    s.serialize(meta.model->platform.id);
    s.serialize(meta.model->id);
}

template <typename D> void deserialize(D &d, SerializedRom &meta) {
    d.deserialize(meta.id);
    d.deserialize(meta.name);
    d.deserialize(meta.platformId);
    d.deserialize(meta.modelId);
}
} // namespace serialize

RomMeta Roms::loadRom(const fs::path &path) {
    SerializedRom sRom;

    std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open ROM file " + path.string());
    }

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    using InputAdapter = serialize::InputBufferAdapter<decltype(buffer)>;
    serialize::Deserializer<InputAdapter> des(buffer);
    des.load(sRom);

    RomMeta meta;
    meta.id = std::move(sRom.id);
    meta.name = std::move(sRom.name);

    lt::PlatformPtr platform = definitions_.fromId(sRom.platformId);
    if (!platform) {
        throw std::runtime_error("ROM " + meta.id +
                                 " has invalid platform id \"" +
                                 sRom.platformId + "\"");
    }

    lt::ModelPtr model = platform->findModel(sRom.modelId);
    if (!model) {
        throw std::runtime_error("ROM " + meta.id + " has invalid model id \"" +
                                 sRom.modelId + "\"");
    }
    meta.model = std::move(model);

    return meta;
}

void Roms::createPath() {
    if (!fs::exists(path_ / "romdata")) {
        fs::create_directories(path_ / "romdata");
    }
}

QModelIndex Roms::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex Roms::parent(const QModelIndex & /*child*/) const {
    return QModelIndex();
}

int Roms::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return roms_.size();
    }

    return 0;
}

int Roms::columnCount(const QModelIndex & /*parent*/) const { return 4; }

QVariant Roms::data(const QModelIndex &index, int role) const {
    if (index.parent().isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole && role != Qt::UserRole) {
        return QVariant();
    }

    const RomMeta &meta = roms_[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: // Name
            return QString::fromStdString(meta.name);
        case 1: // Id
            return QString::fromStdString(meta.id);
        case 2: // Platform
            return QString::fromStdString(meta.model->platform.name);
        case 3: // Model
            return QString::fromStdString(meta.model->name);
        default:
            return QVariant();
        }
    } else if (role == Qt::UserRole) {
        return QVariant();
    }
    return QVariant();
}

QVariant Roms::headerData(int section, Qt::Orientation orientation,
                          int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Name");
    case 1:
        return tr("Id");
    case 2:
        return tr("Platform");
    case 3:
        return tr("Model");
    default:
        return QVariant();
    }
}

void Roms::loadRoms() {
    if (!fs::exists(path_)) {
        return;
    }

    beginResetModel();
    for (auto &sub : fs::directory_iterator(path_)) {
        if (sub.is_regular_file() &&
            sub.path().extension() == fs::path(".lts")) {
            roms_.emplace_back(loadRom(sub.path()));
        }
    }
    endResetModel();
}

void Roms::addRom(const lt::RomPtr &rom) {
    createPath();
    RomMeta meta;
    meta.id = rom->id();
    meta.model = rom->model();
    meta.name = rom->name();

    saveRom(meta);

    beginInsertRows(QModelIndex(), roms_.size(), roms_.size());
    roms_.emplace_back(std::move(meta));
    endInsertRows();
}

void Roms::addRom(const lt::Platform &platform, const std::string &id,
                  const std::string &name, const uint8_t *data,
                  const std::size_t size) {
    lt::ModelPtr model = platform.identify(data, size);
    if (!model) {
        throw std::runtime_error("model could not be identified");
    }

    lt::RomPtr rom = std::make_shared<lt::Rom>(model);
    rom->setData(std::vector(data, data + size));
    rom->setId(id);
    rom->setName(name);

    // Add metadata
    addRom(rom);
    // Save data
    saveRom(rom);
}

void Roms::saveRom(const RomMeta &meta) {
    createPath();

    using OutputAdapter = serialize::OutputBufferAdapter<std::vector<uint8_t>>;

    std::vector<uint8_t> buffer;
    serialize::Serializer<OutputAdapter> ser(buffer);

    ser.save(meta);

    std::ofstream file(path_ / meta.id, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file while saving rom " +
                                 meta.id);
    }

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    file.close();
}

lt::RomPtr Roms::openRom(const RomMeta &meta) const {
    fs::path path = path_ / "romdata" / meta.id;

    auto rom = std::make_shared<lt::Rom>(meta.model);
    rom->setId(meta.id);
    rom->setName(meta.name);

    std::ifstream file(path, std::ios::binary | std::ios::in | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open rom " + path.string());
    }

    auto fileSize = file.tellg();
    std::vector<uint8_t> data(fileSize);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(data.data()), fileSize);
    file.close();

    rom->setData(std::move(data));

    return rom;
}

void Roms::saveRom(const lt::RomPtr &rom) {
    createPath();
    fs::path path = path_ / "romdata" / rom->id();

    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open rom data for saving " + path.string());
    }

    file.write(reinterpret_cast<const char *>(rom->data()), rom->size());
    file.close();
}

RomMeta *Roms::fromId(const std::string &id) noexcept {
    for (RomMeta &rom : roms_) {
        if (rom.id == id) {
            return &rom;
        }
    }
    return nullptr;
}

const RomMeta *Roms::fromId(const std::string &id) const noexcept {
    for (const RomMeta &rom : roms_) {
        if (rom.id == id) {
            return &rom;
        }
    }
    return nullptr;
}
