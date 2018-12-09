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

#include "rom.h"
#include "definitions/definition.h"
#include "libretuner.h"
#include "logger.h"
#include "table.h"

#include <cassert>
#include <fstream>

#include <QFile>


Rom::Rom() {
}



RomData::RomData(Rom &rom) : rom_(rom)
{
    std::ifstream file(rom_.path(), std::ios::binary);
    if (!file.is_open()) {
        // File does not exist, abort.
        throw std::runtime_error("ROM '" + rom_.path() + "' does not exist");
    }
    
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    data_.resize(size);
    if (!file.read(reinterpret_cast<char*>(data_.data()), size)) {
        throw std::runtime_error("Failed to read ROM data");
    }
}



TableAxis * RomData::getAxis(const std::string& name)
{
    {
        auto it = axes_.find(name);
        if (it != axes_.end()) {
            return it->second.get();
        }
    }
    auto it = rom_.platform()->axes.find(name);
    if (it == rom_.platform()->axes.end()) {
        return nullptr;
    }
    
    if (it->second.type == definition::AxisType::Linear) {
        
        std::unique_ptr<TableAxis> axis;
        switch (it->second.dataType) {
            case TableType::Float:
                axis = std::make_unique<LinearAxis<float>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Uint8:
                axis = std::make_unique<LinearAxis<uint8_t>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Uint16:
                axis = std::make_unique<LinearAxis<uint16_t>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Uint32:
                axis = std::make_unique<LinearAxis<uint32_t>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Int8:
                axis = std::make_unique<LinearAxis<int8_t>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Int16:
                axis = std::make_unique<LinearAxis<int16_t>>(it->second.name, it->second.start, it->second.increment);
                break;
            case TableType::Int32:
                axis = std::make_unique<LinearAxis<int32_t>>(it->second.name, it->second.start, it->second.increment);
                break;
        }
        
        TableAxis *ptr = axis.get();
        axes_.emplace(name, std::move(axis));
        return ptr;
    }
    return nullptr;
}



void RomData::setData(std::vector<uint8_t> && data)
{
    data_ = std::move(data);
    
    std::ofstream file(rom_.path(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open '" + rom_.path() + "' for reading");
        return;
    }
    
    file.write(reinterpret_cast<const char*>(data_.data()), data_.size());
}



std::shared_ptr<RomData> Rom::data()
{
    if (auto d = data_.lock()) {
        return d;
    }
    
    std::shared_ptr<RomData> data = std::make_shared<RomData>(*this);
    data_ = data;
    return data;
}



std::unique_ptr<Table> RomData::loadTable(std::size_t tableId)
{
    if (tableId >= rom_.platform()->tables.size()) {
        return nullptr;
    }
    
    const definition::Table &tableDef = rom_.platform()->tables[tableId];
    std::size_t offset = rom_.model()->getOffset(tableId);
    
    // Verify the size of the rom
    auto begin = data_.begin() + offset;
    auto end = begin + tableDef.rawSize();
    
    if (end > data_.end()) {
        throw std::runtime_error("end of table exceeds ROM data");
    }

    return deserializeTable(tableDef, rom_.platform()->endianness, begin, end, getAxis(tableDef.axisX), getAxis(tableDef.axisY), offset);
    
    assert(false && "loadTable() unimplemented");
}



std::shared_ptr<TuneData> Tune::data()
{
    assert(base_);
    if (auto d = data_.lock()) {
        return d;
    }
    
    std::shared_ptr<TuneData> data = std::make_shared<TuneData>(path_, *base_);
    data_ = data;
    return data;
}



Flashable TuneData::flashable()
{
    std::vector<uint8_t> data = baseData_->data();

    // Apply tune on top of ROM
    apply(data.data(), data.size());
    
    std::size_t offset = base_.platform()->flashOffset;

    // Reassign to flash region
    data.assign(data.data() + offset,
                data.data() + offset + base_.platform()->flashSize);
    
    return Flashable(std::move(data), base_.model());
}




Tune::Tune() {
}



Tune *Rom::getTune(std::size_t index)
{
    if (index <= tunes_.size()) {
        return tunes_[index].get();
    }
    return nullptr;
}




void TuneData::readTables(QXmlStreamReader &xml) {
    assert(xml.isStartElement() && xml.name() == "tables");

    while (xml.readNextStartElement()) {
        if (xml.name() != "table") {
            xml.raiseError("Unexpected element in tables");
            return;
        }

        if (!xml.attributes().hasAttribute("id")) {
            xml.raiseError("Expected id attribute");
            return;
        }

        bool valid;
        int id = xml.attributes().value("id").toInt(&valid);
        if (!valid) {
            xml.raiseError("id is not a number");
            return;
        }

        if (id >= tables_.count()) {
            xml.raiseError("id is out of range");
            return;
        }
        
        QByteArray data =
            QByteArray::fromBase64(xml.readElementText().toUtf8());
            
        const definition::Table &definition = base_.platform()->tables[id];
        
        std::unique_ptr<Table> table  = deserializeTable(definition, base_.platform()->endianness, reinterpret_cast<const uint8_t*>(data.begin()), reinterpret_cast<const uint8_t*>(data.end()), baseData_->getAxis(definition.axisX), baseData_->getAxis(definition.axisY), base_.model()->getOffset(id));

        tables_.set(id, std::move(table));
    }
}



void TuneData::save() {
    QFile file(QString::fromStdString(path_));
    if (!file.open(QFile::WriteOnly)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(-1); // tabs > spaces

    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE tune>");
    xml.writeStartElement("tables");

    for (int i = 0; i < tables_.count(); ++i) {
        const Table *table = tables_.get(i, false);
        if (table && table->dirty()) {
            xml.writeStartElement("table");
            xml.writeAttribute("id", QString::number(i));
            std::vector<uint8_t> data;
            data.resize(table->byteSize());
            table->serialize(data.data(), data.size(), base_.platform()->endianness);
            xml.writeCharacters(
                QString(QByteArray(reinterpret_cast<const char *>(data.data()),
                                   data.size())
                            .toBase64()));
            xml.writeEndElement();
        }
    }
    xml.writeEndElement();
    xml.writeEndDocument();

    if (xml.hasError()) {
        throw std::runtime_error("unknown XML error while writing tune");
    }
}



void TuneData::apply(uint8_t *data, size_t size) {
    tables_.apply(data, size, base_.platform()->endianness);

    // Checksums
    base_.model()->checksums.correct(data, size);
}



TuneData::TuneData(std::string path, Rom &base, bool open) : path_(std::move(path)), base_(base), baseData_(base_.data()), tables_(baseData_)
{
    if (open) {
        QFile file(QString::fromStdString(path_));
        if (!file.open(QFile::ReadOnly)) {
            throw std::runtime_error(file.errorString().toStdString());
        }

        QXmlStreamReader xml(&file);
        if (xml.readNextStartElement()) {
            if (xml.name() == "tables") {
                readTables(xml);
            } else {
                xml.raiseError(QObject::tr("Unexpected element"));
            }
        }

        if (xml.error()) {
            throw std::runtime_error(QString("%1\nLine %2, column %3")
                                        .arg(xml.errorString())
                                        .arg(xml.lineNumber())
                                        .arg(xml.columnNumber())
                                        .toStdString());
        }
    }
}
