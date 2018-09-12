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

#include "definition.h"
#include "piddefinitions.h"
#include "tabledefinitions.h"

#include <QDir>

#include <algorithm>
#include <cassert>

namespace definition {

void Model::load(const toml::table& file) {
    name = toml::get<std::string>(file.at("name"));
    id = toml::get<std::string>(file.at("id"));
    
    // Load tables
    const auto &tables = toml::get<std::vector<toml::table>>(file.at("table"));
    std::for_each(tables.begin(), tables.end(), [&](const auto &table) { loadTable(table); });
    
    // Load axes
    const auto &axes = toml::get<std::vector<toml::table>>(file.at("axis"));
    std::for_each(axes.begin(), axes.end(), [&](const auto &axis) { loadAxis(axis); });
    
    // Load identifiers
    const auto &identifiers = toml::get<std::vector<toml::table>>(file.at("identifier"));
    std::for_each(identifiers.begin(), identifiers.end(), [&](const auto &identifier) { loadIdentifier(identifier); });
    
    // Load checksums
    const auto &checksums = toml::get<std::vector<toml::table>>(file.at("checksum"));
    std::for_each(checksums.begin(), checksums.end(), [&](const auto &checksum) { loadChecksum(checksum); });
}



void Model::loadTable(const toml::table &table) {
    const auto id = toml::get<std::size_t>(table.at("id"));
    const auto offset = toml::get<std::size_t>(table.at("offset"));
    
    if (tables.size() <= id) {
        tables.resize(id + 1);
    }
    tables[id] = offset;
}



void Model::loadAxis(const toml::table &axis) {
    const auto &id = toml::get<std::string>(axis.at("id"));
    const auto offset = toml::get<std::size_t>(axis.at("offset"));
    
    axisOffsets.emplace(id, offset);
}



void Model::loadIdentifier(const toml::table &identifier) {
    const auto offset = toml::get<std::size_t>(identifier.at("offset"));
    const auto &data = toml::get<std::string>(identifier.at("data"));
    
    identifiers.emplace_back(offset, data.begin(), data.end());
}



void Model::loadChecksum(const toml::table &checksum) {
    const auto &mode = toml::get<std::string>(checksum.at("mode"));
    const auto offset = toml::get<std::size_t>(checksum.at("offset"));
    const auto size = toml::get<std::size_t>(checksum.at("size"));
    const auto target = toml::get<std::size_t>(checksum.at("target"));
    
    Checksum *sum;
    if (mode == "basic") {
        sum = checksums.addBasic(offset, size, target);
    } else {
        throw std::runtime_error("invalid mode for checksum");
    }
    
    for (const auto &modify : toml::get<std::vector<toml::table>>(checksum.at("modify"))) {
        sum->addModifiable(toml::get<std::size_t>(modify.at("offset")), toml::get<std::size_t>(modify.at("size")));
    }
}



Model::Model(const Main& m) : main(m)
{
}



void Main::load(const std::string& dirPath)
{
    QDir dir(QString::fromStdString(dirPath));

    if (QFile::exists(QString::fromStdString(dirPath + "/main.xml"))) {
        std::ifstream file(dirPath + "/main.xml");
        if (!file.good()) {
            throw std::runtime_error("failed to open " + dirPath + "/main.xml");
        }
        load(toml::parse(file));
    } else {
        throw std::runtime_error(std::string("No main.xml file in ") + dirPath);
    }

    for (QFileInfo &info :
        dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::NoSort)) {
        if (info.isFile()) {
            if (info.fileName().toLower() != "main.xml") {
                // Model
                ModelPtr model = std::make_shared<Model>(*this);
                std::ifstream file(info.filePath().toStdString());
                if (!file.good()) {
                    throw std::runtime_error("failed to open " + dirPath + "/main.xml");
                }
                model->load(toml::parse(file));
            }
        }
    }
}



void Main::load(const toml::table& file)
{
    name = toml::get<std::string>(file.at("name"));
    id = toml::get<std::string>(file.at("id"));
    romsize = toml::get<std::size_t>(file.at("romsize"));
    baudrate = toml::get<std::size_t>(file.at("baudrate"));
    
    // Load transfer
    {
        const auto &transfer = toml::get<toml::table>(file.at("transfer"));
        flashMode = [&](std::string mode) {
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            if (mode == "mazdat1") {
                return FlashMode::T1;
            }
            return FlashMode::None;
        }(toml::get<std::string>(transfer.at("flashmode")));
        
        downloadMode = [&](std::string mode) {
            std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
            if (mode == "mazda23") {
                return DownloadMode::Mazda23;
            }
            return DownloadMode::None;
        }(toml::get<std::string>(transfer.at("downloadmode")));
        
        key = toml::get<std::string>(transfer.at("key"));
        serverId = toml::get<std::size_t>(transfer.at("serverid"));
    }
    
    // Load VIN patterns
    for (const auto &vin : toml::get<std::vector<std::string>>(file.at("vins"))) {
        vins.emplace_back(vin);
    }
    
    // Load tables
    for (const auto &table : toml::get<std::vector<toml::table>>(file.at("table"))) {
        loadTable(table);
    }
    
    // Load axes
    for (const auto &axis : toml::get<std::vector<toml::table>>(file.at("axis"))) {
        loadAxis(axis);
    }
}



void Main::loadTable(const toml::table& table)
{
    Table definition;
    definition.id = toml::get<std::size_t>(table.at("id"));
    
    definition.name = toml::get<std::string>(table.at("name"));
    definition.description = toml::get<std::string>(table.at("description"));
    
    const auto &category = [&] (const toml::value &v) {
        if (v.is<toml::string>()) {
            return toml::get<std::string>(v);
        }
        return std::string("Miscellaneous");
    }(table.at("category"));
    
    definition.dataType = [&](const std::string &type) {
        if (type == "float") {
            return TableType::Float;
        }
        if (type == "uint8") {
            return TableType::Uint8;
        }
        if (type == "uint16") {
            return TableType::Uint16;
        }
        if (type == "uint32") {
            return TableType::Uint32;
        }
        if (type == "int8") {
            return TableType::Int8;
        }
        if (type == "int16") {
            return TableType::Int16;
        }
        if (type == "int32") {
            return TableType::Int32;
        }
        
        throw std::runtime_error("invalid datatype");
    }(toml::get<std::string>(table.at("datatype")));
    
    const auto opt_size = [&](const toml::value &v) -> std::size_t {
        if (v.is<toml::integer>()) {
            return toml::get<std::size_t>(v);
        }
        return 1;
    };
    

    definition.sizeX = opt_size(table.at("sizex"));
    definition.sizeY = opt_size(table.at("sizey"));
    
    const auto opt_string = [&](const toml::value &v) {
        if (v.is<toml::string>()) {
            return toml::get<std::string>(v);
        }
        return std::string();
    };
    
    definition.axisXId = opt_string(table.at("axisx"));
    definition.axisYId = opt_string(table.at("axisy"));
    
    const auto opt_double = [&](const toml::value &v, double def) {
        if (v.is<double>()) {
            return toml::get<double>(v);
        }
        return def;
    };
    
    definition.minimum = opt_double(table.at("minimum"), std::numeric_limits<double>::min());
    definition.maximum = opt_double(table.at("maximum"), std::numeric_limits<double>::max());
    
    tables.emplace_back(std::move(definition));
}



void Main::loadAxis(const toml::table& axis)
{
    Axis definition;
    
    definition.name = toml::get<std::string>(axis.at("name"));
    definition.id = toml::get<std::string>(axis.at("id"));
    
    const std::string &type = toml::get<std::string>(axis.at("type"));
    if (type == "linear") {
        double start = toml::get<double>(axis.at("minimum"));
        double increment = toml::get<double>(axis.at("increment"));
        
        axes.emplace(definition.id, std::make_unique<LinearAxis>(start, increment));
    } else {
        throw std::runtime_error("invalid axis type");
    }
}



LinearAxis::LinearAxis(double start, double increment) : start_(start), increment_(increment)
{
}



double LinearAxis::label(std::size_t idx) const
{
    return start_ + (idx * increment_);
}



std::size_t Table::rawSize() const
{
    return tableTypeSize(type) * sizeX * sizeY;
}


/*
uint32_t SubDefinition::getAxisLocation(int axisId, bool *ok) {
    assert(axisId >= 0);
    if (axisId >= axesOffsets_.size()) {
        if (ok != nullptr) {
            *ok = false;
        }
        return 0;
    }

    if (ok != nullptr) {
        *ok = true;
    }
    return axesOffsets_[axisId];
}

bool SubDefinition::check(gsl::span<const uint8_t> data) {
    for (Identifier &identifier : identifiers_) {
        if (identifier.offset() + identifier.size() > data.size()) {
            return false;
        }

        if (std::equal(data.begin() + identifier.offset(), data.end(),
                       identifier.data(),
                       identifier.data() + identifier.size()) != 0) {
            return false;
        }
    }
    return true;
}

bool SubDefinition::load(const QString &path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        lastError_ = "Failed to open definition file for reading";
        return false;
    }

    QXmlStreamReader xml(&file);

    if (xml.readNextStartElement()) {
        if (xml.name() != "subdefinition") {
            xml.raiseError("Unexpected element");
        }
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == "id") {
            id_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "name") {
            name_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "tables") {
            loadTables(xml);
        } else if (xml.name() == "axes") {
            loadAxes(xml);
        } else if (xml.name() == "checksums") {
            loadChecksums(xml);
        } else {
            xml.raiseError("Unexpected element");
        }
    }

    if (!xml.hasError() && id_.empty()) {
        xml.raiseError("No id element is defined");
    }

    if (xml.hasError()) {
        lastError_ = QObject::tr("%1\nLine %2, column %3")
                         .arg(xml.errorString())
                         .arg(xml.lineNumber())
                         .arg(xml.columnNumber())
                         .toStdString();
        return false;
    }
    return true;
}

int Definition::axisId(const std::string &id) {
    auto it = axes_.find(id);
    if (it == std::end(axes_)) {
        return -1;
    }
    return it->second->iId();
}

SubDefinitionPtr Definition::identifySubtype(gsl::span<const uint8_t> data) {
    for (SubDefinitionPtr &def : subtypes_) {
        if (def->check(data)) {
            return def;
        }
    }
    return nullptr;
}

void Definition::readTables(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "table") {
            xml.raiseError("Unknown element");
            break;
        }

        TableDefinition definition(this);
        if (definition.load(xml, this)) {
            tables_.addTable(std::move(definition));
        }
    }
}

void Definition::loadAxes(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "axis") {
            xml.raiseError("Unexpected element");
            return;
        }

        TableAxisPtr axis = TableAxis::load(xml, lastAxisId_++);
        if (!axis) {
            return;
        }

        axes_.insert(
            std::make_pair<std::string, TableAxisPtr &>(axis->id(), axis));
    }
}

void Definition::loadVins(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "vin") {
            xml.raiseError("Unexpected element");
            return;
        }

        try {
            vins_.emplace_back(xml.readElementText().trimmed().toStdString());
        } catch (const std::regex_error &e) {
            xml.raiseError(QStringLiteral("Could not load vin regex: ") +
                           e.what());
        }
    }
}

void Definition::loadPids(QXmlStreamReader &xml) {
    while (xml.readNextStartElement()) {
        if (xml.name() != "pid") {
            xml.raiseError("Unexpected element");
            return;
        }

        PidDefinition pid;
        pid.valid = true;
        QXmlStreamAttributes attributes = xml.attributes();
        if (!attributes.hasAttribute("id")) {
            xml.raiseError("Could not load PID: no id attribute");
            return;
        }
        bool ok;
        pid.id = attributes.value("id").toUInt(&ok);
        if (!ok) {
            xml.raiseError(
                "Could not load PID: id attribute is not an integer");
            return;
        }

        while (xml.readNextStartElement()) {
            if (xml.name() == "name") {
                pid.name = xml.readElementText().toStdString();
            } else if (xml.name() == "description") {
                pid.description = xml.readElementText().toStdString();
            } else if (xml.name() == "formula") {
                pid.formula = xml.readElementText().toStdString();
            } else if (xml.name() == "unit") {
                pid.unit = xml.readElementText().toStdString();
            } else if (xml.name() == "code") {
                pid.code = xml.readElementText().toUInt(&ok, 16);
                if (!ok) {
                    xml.raiseError(
                        "Could not load PID: code is not an integer");
                    return;
                }
            }
        }

        pids_.add(std::move(pid));
    }
}

bool Definition::loadMain(const QString &path) {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        lastError_ = "Failed to open main definition file for reading";
        return false;
    }

    QXmlStreamReader xml(&file);

    bool foundSize = false, foundFlashOffset = false, foundFlashSize = false;

    // Default endianness is big
    endianness_ = Endianness::Big;
    downloadMode_ = DM_NONE;
    flashMode_ = FLASH_NONE;

    if (xml.readNextStartElement()) {
        if (xml.name() != "maindefinition") {
            xml.raiseError("Unexpected element");
        }
    }

    bool ok;
    while (xml.readNextStartElement()) {
        if (xml.name() == "name") {
            name_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "id") {
            id_ = xml.readElementText().trimmed().toStdString();
        } else if (xml.name() == "romsize") {
            size_ = xml.readElementText().toUInt(&ok);
            if (!ok) {
                xml.raiseError("Invalid romsize: not a number");
            }
            foundSize = true;
        } else if (xml.name() == "endianness") {
            QString sEndian = xml.readElementText().trimmed().toLower();
            if (sEndian == "big") {
                endianness_ = Endianness::Big;
            } else if (sEndian == "little") {
                endianness_ = Endianness::Little;
            } else {
                xml.raiseError(
                    "Invalid endianness. Expected values: 'big' or 'little'");
            }
        } else if (xml.name() == "tables") {
            readTables(xml);
        } else if (xml.name() == "transfer") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "flashmode") {
                    QString sMode = xml.readElementText().toLower();
                    if (sMode == "mazdat1") {
                        flashMode_ = FLASH_T1;
                    } else {
                        xml.raiseError("Invalid flash mode");
                    }
                } else if (xml.name() == "downloadmode") {
                    QString sMode = xml.readElementText().toLower();
                    if (sMode == "mazda23") {
                        downloadMode_ = DM_MAZDA23;
                    } else {
                        xml.raiseError("Invalid download mode");
                    }
                } else if (xml.name() == "key") {
                    key_ = xml.readElementText().toStdString();
                } else if (xml.name() == "serverid") {
                    serverId_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid server id: not a number");
                    }
                } else {
                    xml.raiseError("Unexpected element");
                }
            }
        } else if (xml.name() == "axes") {
            loadAxes(xml);
        } else if (xml.name() == "flashregion") {
            while (xml.readNextStartElement()) {
                if (xml.name() == "offset") {
                    flashOffset_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid flash offset: not a number");
                    }
                    foundFlashOffset = true;
                } else if (xml.name() == "size") {
                    flashSize_ = xml.readElementText().toUInt(&ok, 16);
                    if (!ok) {
                        xml.raiseError("Invalid flash size: not a number");
                    }
                    foundFlashSize = true;
                }
            }
        } else if (xml.name() == "vins") {
            loadVins(xml);
        } else if (xml.name() == "pids") {
            loadPids(xml);
        } else if (xml.name() == "baudrate") {
            baudrate_ = xml.readElementText().toUInt(&ok);
            if (!ok) {
                xml.raiseError("Invalid baudrate: not a number");
            }
        } else if (xml.name() == "logmode") {
            QString sMode = xml.readElementText().toLower();
            if (sMode == "uds") {
                logMode_ = LogMode::Uds;
            } else {
                xml.raiseError("Unknown log mode");
            }
        } else {
            xml.raiseError("Unexpected element");
        }
    }

    if (!xml.hasError()) {
        if (id_.empty()) {
            xml.raiseError("No id element is defined");
        } else if (!foundSize) {
            xml.raiseError("No romsize element is defined");
        } else if (!foundFlashOffset) {
            xml.raiseError("No flash offset defined");
        } else if (!foundFlashSize) {
            xml.raiseError("No flash size defined");
        } else {
            // Ensure that the flash region is in bounds
            if (flashOffset_ + flashSize_ > size_) {
                xml.raiseError("Flash region is out of bounds");
            }
        }
    }

    if (xml.hasError()) {
        lastError_ =
            (QObject::tr("error loading ") + path + ": %1\nLine %2, column %3")
                .arg(xml.errorString())
                .arg(xml.lineNumber())
                .arg(xml.columnNumber())
                .toStdString();
        return false;
    }
    return true;
}

SubDefinitionPtr Definition::findSubtype(const std::string &id) {
    auto it = std::find_if(
        subtypes_.begin(), subtypes_.end(),
        [id](SubDefinitionPtr &def) -> bool { return def->id() == id; });
    if (it == subtypes_.end()) {
        return nullptr;
    }

    return *it;
}

bool Definition::loadSubtype(const QString &path) {
    SubDefinitionPtr sub = std::make_shared<SubDefinition>(this);
    if (!sub->load(path)) {
        lastError_ = std::string("Could not load definition file ") +
                     path.toStdString() + ": " + sub->lastError();
        return false;
    }

    subtypes_.push_back(sub);
    return true;
}

TableAxis *Definition::getAxis(const std::string &id) {
    auto it = axes_.find(id);
    if (it == std::end(axes_)) {
        return nullptr;
    }
    return it->second.get();
}

bool Definition::load(const QString &path) {
    QDir dir(path);

    Definition definition;

    if (QFile::exists(path + "/main.xml")) {
        if (!loadMain(path + "/main.xml")) {
            return false;
        }
    } else {
        lastError_ = std::string("No main.xml file in ") + path.toStdString();
        return false;
    }

    for (QFileInfo &info :
         dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::NoSort)) {
        if (info.isFile()) {
            if (info.fileName().toLower() != "main.xml") {
                // Subtype
                if (!loadSubtype(info.filePath())) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Definition::matchVin(const std::string &vin) {
    return std::any_of(vins_.begin(), vins_.end(), [&vin](const auto &pattern) {
        return std::regex_match(vin, pattern);
    });
}
*/

}
