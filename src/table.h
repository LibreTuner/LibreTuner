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
#include <cmath>
#include <sstream>
#include <iomanip>

#include <QAbstractTableModel>
#include <QFontDatabase>
#include <QApplication>
#include <QColor>
#include <QFont>


class TableAxis
 {
public:
    virtual double label(int index) const =0;
    virtual const std::string &name() const =0;
    virtual ~TableAxis();
};


template<typename DataType>
class MemoryAxis : public TableAxis
{
public:
    template<typename InputIt>
    MemoryAxis(std::string name, InputIt begin, InputIt end, Endianness endianness);
    
    virtual double label(int index) const override;
    virtual const std::string &name() const override;
    
private:
    std::vector<DataType> data_;
    std::string name_;
};

template<typename DataType>
template<typename InputIt>
MemoryAxis<DataType>::MemoryAxis(std::string name, InputIt begin, InputIt end, Endianness endianness) : name_(name)
{
    switch (endianness) {
    case Endianness::Big:
        for (auto it = begin; it < end; it += sizeof(DataType)) {
            DataType data = readBE<DataType>(it, end);
            data_.emplace_back(data);
        }
        break;
    case Endianness::Little:
        for (auto it = begin; it < end; it += sizeof(DataType)) {
            DataType data = readLE<DataType>(it, end);
            data_.emplace_back(data);
        }
        break;
    }
}

template<typename DataType>
double MemoryAxis<DataType>::label(int index) const
{
    if (index < 0 || index >= data_.size()) {
        Logger::warning("Axis index '" + std::to_string(index) + "' exceeds axis size (" + std::to_string(data_.size()) + ")");
        return 0;
    }
    
    return data_[index];
}


template<typename DataType>
const std::string &MemoryAxis<DataType>::name() const {
    return name_;
}


template<typename T>
class LinearAxis : public TableAxis
{
public:
    LinearAxis(std::string name, T start, T increment);

    virtual double label(int index) const override;
    virtual const std::string &name() const override;
private:
    T start_, increment_;
    std::string name_;
};



template<typename T>
LinearAxis<T>::LinearAxis(std::string name, T start, T increment) : start_(start), increment_(increment), name_(std::move(name)) {

}


template<typename T>
double LinearAxis<T>::label(int index) const {
    return (start_) + index * increment_;
}


template<typename T>
const std::string &LinearAxis<T>::name() const
{
    return name_;
}




class Table : public QAbstractTableModel {
Q_OBJECT
public:
    Table() {}
    
    virtual ~Table() override = default;
    
    virtual std::size_t width() const =0;
    virtual std::size_t height() const =0;
    
    // Returns true if the data at (x, y) has been modified
    virtual bool modified(std::size_t x, std::size_t y) const =0;
    
    virtual bool dirty() const =0;
    
    virtual void serialize(uint8_t *data, size_t len, Endianness endianness) const =0;
    
    virtual TableType dataType() const =0;
    
    virtual std::size_t byteSize() const =0;
    
    virtual TableAxis *axisX() const =0;
    virtual TableAxis *axisY() const =0;
    virtual std::size_t offset() const =0;
    
    virtual const std::string name() const =0;
    virtual const std::string description() const =0;
    virtual const double maximum() const =0;
    virtual const double minimum() const =0;
    virtual const definition::Table& definition() const =0;
    
    bool isTwoDimensional() const { return height() > 1; }
    bool isOneDimensional() const { return height() == 1; }
    
    // QAbstractTableModel overrides
    int columnCount(const QModelIndex & parent) const override { return width(); }
    int rowCount(const QModelIndex & parent) const override { return height(); }
    
    Qt::ItemFlags flags(const QModelIndex &index) const override { return Qt::ItemFlags(Qt::ItemIsEnabled) | Qt::ItemIsSelectable | Qt::ItemIsEditable; }

signals:
    void onModified();
};


template<typename DataType>
struct TableInfo {
    std::size_t width;
    DataType minimum;
    DataType maximum;
    TableAxis *axisX = nullptr;
    TableAxis *axisY = nullptr;
    std::size_t offset;
};


template<typename DataType>
class TableStore {
public:
    virtual DataType get(std::size_t index) const =0;
    virtual void set(std::size_t index, DataType data) =0;
    virtual std::size_t size() const =0;

    virtual void serialize(uint8_t *buffer, size_t len, Endianness endianness) const =0;

    virtual std::size_t byteSize() const =0;
};

// This design results in 49 definitions (for each type of datatype * each type of stored datatype)
// If should be changed in the future. At the moment, I do not want to mess with these gross templates
template<typename DataType, typename StoredDataType>
class TableStoreBase : public TableStore<DataType> {
public:
    template<typename InputIt>
    TableStoreBase(InputIt begin, InputIt end, Endianness endianness, double scale);

    virtual DataType get(std::size_t index) const override;
    virtual void set(std::size_t index, DataType data) override;
    virtual std::size_t size() const override;

    virtual void serialize(uint8_t *buffer, size_t len, Endianness endianness) const override;

    virtual std::size_t byteSize() const override;


private:
    std::vector<StoredDataType> data_;
    double scale_;
};



template<typename DataType, typename StoredDataType>
DataType TableStoreBase<DataType, StoredDataType>::get(std::size_t index) const
{
    return static_cast<DataType>(data_[index] * scale_);
}



template<typename DataType, typename StoredDataType>
void TableStoreBase<DataType, StoredDataType>::serialize(uint8_t* buffer, size_t len, Endianness endianness) const
{
    if (len < byteSize()) {
        throw std::runtime_error("data array is too small to serialize table into");
    }

    uint8_t *end = buffer + len;

    if (endianness == Endianness::Big) {
        for (StoredDataType data : data_) {
            writeBE<StoredDataType>(data, buffer, end);
            buffer += sizeof(DataType);
        }
    } else {
        // Little endian
        for (StoredDataType data : data_) {
            writeLE<StoredDataType>(data, buffer, end);
            buffer += sizeof(DataType);
        }
    }
}



template<typename DataType, typename StoredDataType>
void TableStoreBase<DataType, StoredDataType>::set(std::size_t index, DataType data)
{
    data_[index] = static_cast<StoredDataType>(data / scale_);
}



template<typename DataType, typename StoredDataType>
std::size_t TableStoreBase<DataType, StoredDataType>::size() const
{
    return data_.size();
}



template<typename DataType, typename StoredDataType>
std::size_t TableStoreBase<DataType, StoredDataType>::byteSize() const
{
    return data_.size() * sizeof(StoredDataType);
}



template<typename DataType, typename StoredDataType>
template<typename InputIt>
TableStoreBase<DataType, StoredDataType>::TableStoreBase(InputIt begin, InputIt end, Endianness endianness, double scale) : scale_(scale)
{
    // Start deserializing
    switch (endianness) {
    case Endianness::Big:
        for (auto it = begin; it < end; it += sizeof(StoredDataType)) {
            StoredDataType data = readBE<StoredDataType>(it, end);
            data_.emplace_back(data);
        }
        break;
    case Endianness::Little:
        for (auto it = begin; it < end; it += sizeof(StoredDataType)) {
            StoredDataType data = readLE<StoredDataType>(it, end);
            data_.emplace_back(data);
        }
        break;
    }
}


template<typename DataType>
class TableBase : public Table {
public:
    TableBase(const definition::Table& definition, std::unique_ptr<TableStore<DataType>> &&data, const TableInfo<DataType> &info);
    TableBase(std::size_t width, std::size_t height);

    std::size_t height() const override { return data_->size() / width_; }
    std::size_t width() const override { return width_; }

    void serialize(uint8_t *data, size_t len, Endianness endianness) const override;

    DataType get(std::size_t x, std::size_t y) const;
    void set(std::size_t x, std::size_t y, DataType data);
    bool modified(std::size_t x, std::size_t y) const override;
    bool dirty() const override { return dirty_; }
    TableType dataType() const override;
    std::size_t byteSize() const override;
    virtual TableAxis *axisX() const override { return axisX_; }
    virtual TableAxis *axisY() const override { return axisY_; }
    std::size_t offset() const override { return offset_; }

    const std::string name() const override { return definition_.name; }
    const std::string description() const override { return definition_.description; }
    const double maximum() const override { return definition_.maximum; }
    const double minimum() const override { return definition_.minimum; }
    const definition::Table& definition() const override { return definition_; }

    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

private:
    std::unique_ptr<TableStore<DataType>> data_;
    std::vector<bool> modified_;
    bool dirty_ {false};
    std::size_t width_;
    DataType minimum_, maximum_;
    TableAxis *axisX_;
    TableAxis *axisY_;
    std::size_t offset_;
    const definition::Table& definition_;
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
    data_->serialize(buffer, len, endianness);
    /*if (len < byteSize()) {
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
    }*/
}



template<typename DataType>
inline std::size_t TableBase<DataType>::byteSize() const
{
    return data_->byteSize();
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
QVariant TableBase<DataType>::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || section < 0) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (!axisX_) {
            return QVariant();
        }

        if (section >= width()) {
            return QVariant();
        }

        return round(axisX_->label(section) * 100.0) / 100.0;
    } else {
        if (!axisY_) {
            return QVariant();
        }

        if (section >= height()) {
            return QVariant();
        }

        return round(axisY_->label(section) * 100.0) / 100.0;
    }
}


template<typename DataType>
QVariant TableBase<DataType>::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.column() < 0 || index.row() >= height() || index.column() >= width()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QString::number(get(index.column(), index.row()));
    }

    if (role == Qt::FontRole) {
        QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        if (modified(index.column(), index.row())) {
            // Value has been modified
            font.setWeight(QFont::Bold);
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

    return QVariant();
}


template<class InputIt>
class TableDeserializer {
public:
    TableDeserializer(const definition::Table& definition, Endianness endianness, InputIt begin, InputIt end, TableAxis *axisX, TableAxis *axisY, std::size_t offset) :
        definition_(definition), endianness_(endianness), begin_(begin), end_(end), axisX_(axisX), axisY_(axisY), offset_(offset) {}

    template<typename DataType>
    std::unique_ptr<Table> deserialize() {
        std::unique_ptr<TableStore<DataType>> data;
        switch (definition_.storedDataType) {
            case TableType::Float:
                data = std::make_unique<TableStoreBase<DataType, float>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Uint8:
                data = std::make_unique<TableStoreBase<DataType, uint8_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Uint16:
                data = std::make_unique<TableStoreBase<DataType, uint16_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Uint32:
                data = std::make_unique<TableStoreBase<DataType, uint32_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Int8:
                data = std::make_unique<TableStoreBase<DataType, int8_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Int16:
                data = std::make_unique<TableStoreBase<DataType, int16_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            case TableType::Int32:
                data = std::make_unique<TableStoreBase<DataType, int32_t>>(std::forward<InputIt>(begin_), std::forward<InputIt>(end_), endianness_, definition_.scale);
                break;
            default:
                assert(false && "Unimplemented");
        }
        return std::make_unique<TableBase<DataType>>(definition_, std::move(data), TableInfo<DataType>{definition_.sizeX, static_cast<DataType>(definition_.minimum), static_cast<DataType>(definition_.maximum), axisX_, axisY_, offset_});
    }

private:
    const definition::Table &definition_;
    Endianness endianness_;
    InputIt begin_;
    InputIt end_;
    TableAxis *axisX_, *axisY_;
    std::size_t offset_;
};



template<class InputIt>
std::unique_ptr<Table> deserializeTable(const definition::Table& definition, Endianness endianness, InputIt begin, InputIt end, TableAxis *axisX = nullptr, TableAxis *axisY = nullptr, std::size_t offset = 0)
{
    TableDeserializer<InputIt> deserializer(definition, endianness, begin, end, axisX, axisY, offset);

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
TableBase<DataType>::TableBase(const definition::Table& definition, std::unique_ptr<TableStore<DataType>> &&data, const TableInfo<DataType> &info) : width_(info.width), minimum_(info.minimum), maximum_(info.maximum), axisX_(info.axisX), axisY_(info.axisY), offset_(info.offset), definition_(definition)
{
    data_ = std::move(data);
    modified_.resize(data_->size());
    /*f ((std::distance(begin, end) / sizeof(StoredDataType)) % info.width != 0) {
        throw std::runtime_error("table does not fit in given width (size % width != 0)");
    }*/

    // data_ = std::make_unique<TableStoreBase<DataType, StoredDataType>>(std::move(begin), std::move(end), endianness);
}


template<typename DataType>
inline DataType TableBase<DataType>::get(std::size_t x, std::size_t y) const
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    return data_->get(y * width() + x);
}



template<typename DataType>
inline void TableBase<DataType>::set(std::size_t x, std::size_t y, DataType data)
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    modified_[y * width() + x] = true;
    data_->set(y * width() + x, data);
    dirty_ = true;
    emit onModified();
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
