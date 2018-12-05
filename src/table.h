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

#ifndef TABLE_H
#define TABLE_H

/* Because this file includes function definitions, only include
 * it if absolutely necessary to decrease compile times. */

#include "endian.h"
#include "util.hpp"
#include "enums.hpp"
#include "definitions/definition.h"
#include "logger.h"

#include <cassert>
#include <vector>
#include <memory>

#include <QAbstractTableModel>
#include <QApplication>
#include <QColor>
#include <QFont>



struct TableMeta {
    std::string name;
    std::string description;
};


class Axis {
public:
    virtual std::string label(int index) const =0;
    virtual ~Axis() {}
};

class LinearAxis : public Axis {
public:
    LinearAxis(double start, double increment);

    std::string label(int index) const override;
private:
    double start_, increment_;
};

class Table : public QAbstractTableModel {
public:
    Table(const TableMeta &meta) : meta_(meta) {}
    
    virtual ~Table() override = default;
    
    virtual std::size_t width() const =0;
    virtual std::size_t height() const =0;
    
    // Returns true if the data at (x, y) has been modified
    virtual bool modified(std::size_t x, std::size_t y) const =0;
    
    virtual bool dirty() const =0;
    
    virtual void serialize(uint8_t *data, size_t len, Endianness endianness) const =0;
    
    virtual TableType dataType() const =0;
    
    virtual std::size_t byteSize() const =0;
    
    const std::string name() const { return meta_.name; }
    const std::string description() const { return meta_.description; }
    
    bool isTwoDimensional() const { return height() > 1; }
    bool isOneDimensional() const { return height() == 1; }
    
    // QAbstractTableModel overrides
    int columnCount(const QModelIndex & parent) const override { return width(); }
    int rowCount(const QModelIndex & parent) const override { return height(); }
    
    Qt::ItemFlags flags(const QModelIndex &index) const override { return Qt::ItemFlags(Qt::ItemIsEnabled) | Qt::ItemIsSelectable | Qt::ItemIsEditable; }
    
private:
    TableMeta meta_;
};


template<typename DataType>
struct TableInfo {
    std::size_t width;
    DataType minimum;
    DataType maximum;
    std::shared_ptr<Axis> axisX;
    std::shared_ptr<Axis> axisY;
};


template<typename DataType>
class TableBase : public Table {
public:
    template<class InputIt>
    TableBase(const TableMeta &meta, InputIt begin, InputIt end, Endianness endianness, const TableInfo<DataType> &info);
    TableBase(std::size_t width, std::size_t height);
    
    std::size_t height() const override { return data_.size() / width_; }
    std::size_t width() const override { return width_; }
    
    DataType minimum() const { return minimum_; }
    DataType maximum() const { return maximum_; }
    
    void serialize(uint8_t *data, size_t len, Endianness endianness) const override;
    
    DataType get(std::size_t x, std::size_t y) const;
    void set(std::size_t x, std::size_t y, DataType data);
    bool modified(std::size_t x, std::size_t y) const override;
    bool dirty() const override { return dirty_; }
    TableType dataType() const override;
    std::size_t byteSize() const override;
    
    QVariant data(const QModelIndex & index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    std::vector<DataType> data_;
    std::vector<bool> modified_;
    bool dirty_ {false};
    std::size_t width_;
    DataType minimum_, maximum_;
    std::shared_ptr<Axis> axisX_;
    std::shared_ptr<Axis> axisY_;
};



template<class InputIt>
inline std::unique_ptr<Table> deserializeTable(const definition::Table &definition, Endianness endianness, InputIt begin, InputIt end);



template<>
inline TableType TableBase<float>::dataType() const {
    return TableType::Float;
}



template<>
inline TableType TableBase<uint8_t>::dataType() const {
    return TableType::Uint8;
}



template<>
inline TableType TableBase<uint16_t>::dataType() const {
    return TableType::Uint16;
}



template<>
inline TableType TableBase<uint32_t>::dataType() const {
    return TableType::Uint32;
}



template<>
inline TableType TableBase<int8_t>::dataType() const {
    return TableType::Int8;
}



template<>
inline TableType TableBase<int16_t>::dataType() const {
    return TableType::Int16;
}



template<>
inline TableType TableBase<int32_t>::dataType() const {
    return TableType::Int32;
}



template<typename DataType>
void TableBase<DataType>::serialize(uint8_t *buffer, size_t len, Endianness endianness) const
{
    if (len < byteSize()) {
        throw std::runtime_error("data array is too small to serialize table into");
    }

    uint8_t *end = buffer + len;
    
    if (endianness == Endianness::Big) {
        for (DataType data : data_) {
            writeBE<DataType>(data, buffer, end);
            buffer += sizeof(DataType);
        }
    } else {
        // Little endian
        for (DataType data : data_) {
            writeLE<DataType>(data, buffer, end);
            buffer += sizeof(DataType);
        }
    }
}



template<typename DataType>
inline std::size_t TableBase<DataType>::byteSize() const
{
    return sizeof(DataType) * data_.size();
}



template<typename DataType>
bool TableBase<DataType>::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) {
        return false;
    }
    
    bool success;
    
    if (!value.canConvert<DataType>()) {
        return false;
    }
    
    DataType res = value.value<DataType>();

    set(index.column(), index.row(), res);
    emit dataChanged(index, index);
    return true;
}



template<typename DataType>
QVariant TableBase<DataType>::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.column() < 0 || index.row() >= height() || index.column() >= width()) {
        return QVariant();
    }

    if (role == Qt::FontRole) {
        QFont font = QApplication::font("QTableView");
        if (modified(index.column(), index.row())) {
            // Value has been modified
            font.setWeight(QFont::Medium);
        } else {
            font.setWeight(QFont::Normal);
        }
        return font;
    }
    if (role == Qt::ForegroundRole) {
        if (width() == 1 && height() == 1) {
            // TODO: replace this with a check of the background color
            return QVariant();
        }
        return QColor::fromRgb(0, 0, 0);
    }

    if (role == Qt::BackgroundColorRole) {
        if (width() == 1 && height() == 1) {
            return QVariant();
        }
        double ratio =
            static_cast<double>(get(index.column(), index.row()) - minimum()) /
            (maximum() - minimum());
        if (ratio < 0.0) {
            return QVariant();
        }
        return QColor::fromHsvF((1.0 - ratio) * (1.0 / 3.0), 1.0, 1.0);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return QString::fromStdString(std::to_string(get(index.column(), index.row())));
}


template<class InputIt>
class TableDeserializer {
public:
    TableDeserializer(const definition::Table& definition, Endianness endianness, InputIt begin, InputIt end) :
        definition_(definition), endianness_(endianness), begin_(begin), end_(end) {}

    template<typename T>
    std::unique_ptr<Table> deserialize() {
        return std::make_unique<TableBase<T>>(TableMeta{definition_.name, definition_.description}, std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, TableInfo<T>{definition_.sizeX, definition_.minimum, definition_.maximum, nullptr, nullptr});
    }

private:
    const definition::Table &definition_;
    Endianness endianness_;
    InputIt begin_;
    InputIt end_;
};


template<class InputIt>
std::unique_ptr<Table> deserializeTable(const definition::Table& definition, Endianness endianness, InputIt begin, InputIt end)
{
    TableDeserializer<InputIt> deserializer(definition, endianness, begin, end);

    switch (definition.dataType) {
        case TableType::Float:
            return deserializer.template deserialize<float>();
        case TableType::Uint8:
            return deserializer.template deserialize<uint8_t>();
        case TableType::Uint16:
            return deserializer.template deserialize<uint16_t>();
        case TableType::Uint32:
            return deserializer.template deserialize<uint32_t>();
        case TableType::Int8:
            return deserializer.template deserialize<int8_t>();
        case TableType::Int16:
            return deserializer.template deserialize<int16_t>();
        case TableType::Int32:
            return deserializer.template deserialize<int32_t>();
    }
    return nullptr;
}


template <typename DataType>
template <typename InputIt>
TableBase<DataType>::TableBase(const TableMeta &meta, InputIt begin, InputIt end, Endianness endianness, const TableInfo<DataType> &info) : Table(meta), modified_(std::distance(begin, end) / sizeof(DataType)), width_(info.width), minimum_(info.minimum), maximum_(info.maximum)
{
    if ((std::distance(begin, end) / sizeof(DataType)) % info.width != 0) {
        throw std::runtime_error("table does not fit in given width (size % width != 0)");
    }
    
    // Start deserializing
    switch (endianness) {
    case Endianness::Big:
        for (auto it = begin; it < end; it += sizeof(DataType)) {
            DataType data = readBE<DataType>(it, it + sizeof(DataType));
            data_.emplace_back(data);
        }
        break;
    case Endianness::Little:
        for (auto it = begin; it < end; it += sizeof(DataType)) {
            DataType data = readLE<DataType>(it, it + sizeof(DataType));
            data_.emplace_back(data);
        }
        break;
    }
}



template<typename DataType>
TableBase<DataType>::TableBase(std::size_t width, std::size_t height) : data_(width * height), modified_(width * height), width_(width)
{
}



template<typename DataType>
inline DataType TableBase<DataType>::get(std::size_t x, std::size_t y) const
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    return data_[y * width() + x];
}



template<typename DataType>
inline void TableBase<DataType>::set(std::size_t x, std::size_t y, DataType data)
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    modified_[y * width() + x] = true;
    data_[y * width() + x] = data;
}



template<typename DataType>
inline bool TableBase<DataType>::modified(std::size_t x, std::size_t y) const
{
    if (x >= width() || y >= height()) {
        return false;
    }
    return modified_[y * width() + x];
}


#endif // TABLE_H
