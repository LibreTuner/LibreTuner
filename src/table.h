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

#include <cassert>
#include <vector>
#include <variant>
#include <gsl/span>
#include <memory>

#include <QAbstractTableModel>
#include <QApplication>
#include <QColor>
#include <QFont>


class Table : public QAbstractTableModel {
public:
    virtual std::size_t width() const =0;
    virtual std::size_t height() const =0;
    
    // Returns true if the data at (x, y) has been modified
    virtual bool modified(std::size_t x, std::size_t y) const =0;
    
    virtual TableType dataType() const =0;
    
    // QAbstractTableModel overrides
    int columnCount(const QModelIndex & parent) const override { return width(); }
    int rowCount(const QModelIndex & parent) const override { return height(); }
};



template<typename DataType>
class TableBase : public Table {
public:
    template<class InputIt>
    TableBase(InputIt begin, InputIt end, std::size_t height = 1);
    template<class InputIt>
    TableBase(InputIt begin, InputIt end, Endianness endianness, std::size_t height = 1);
    TableBase(std::size_t width, std::size_t height);
    
    std::size_t height() const override { return data_.size() / width_; }
    std::size_t width() const override { return width_; }
    
    DataType minimum() const { return minimum_; };
    DataType maximum() const { return maximum_; };
    
    DataType get(std::size_t x, std::size_t y) const;
    void set(std::size_t x, std::size_t y, DataType data);
    bool modified(std::size_t x, std::size_t y) const override;
    bool dirty() const { return dirty_; }
    TableType dataType() const override;
    
    QVariant data(const QModelIndex & index, int role) const override;

private:
    std::vector<DataType> data_;
    std::vector<bool> modified_;
    bool dirty_ {false};
    std::size_t width_;
    DataType minimum_, maximum_;
};



template<>
TableType TableBase<float>::dataType() const {
    return TableType::Float;
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
        double ratio =
            static_cast<double>(get(index.column()) - minimum()) /
            (maximum() - minimum());
        if (ratio < 0.0) {
            return QVariant();
        }
        return QColor::fromHsvF((1.0 - ratio) * (1.0 / 3.0), 1.0, 1.0);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return std::to_string(get(index.column(), index.row()));
}



template <typename DataType>
template <typename InputIt>
TableBase<DataType>::TableBase(InputIt begin, InputIt end, std::size_t width) : data_(begin, end), modified_(std::distance(begin, end)), width_(width)
{
    if (std::distance(begin, end) % width == 0) {
        throw std::runtime_error("table does not fit in given width (size % width != 0)");
    }
}



template <typename DataType>
template <typename InputIt>
TableBase<DataType>::TableBase(InputIt begin, InputIt end, Endianness endianness, std::size_t width) : modified_(std::distance(begin, end) / sizeof(DataType)), width_(width)
{
    if ((std::distance(begin, end) / sizeof(DataType)) % width == 0) {
        throw std::runtime_error("table does not fit in given width (size % width != 0)");
    }
    
    // Start deserializing
    switch (endianness) {
    case Endianness::Big:
        for (auto it = begin; it != end; it += sizeof(DataType)) {
            DataType data = readBE<DataType>(gsl::make_span(it, sizeof(DataType)));
            data_.emplace_back(data);
        }
        break;
    case Endianness::Little:
        for (auto it = begin; it != end; it += sizeof(DataType)) {
            DataType data = readLE<DataType>(gsl::make_span(it, sizeof(DataType)));
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
DataType TableBase<DataType>::get(std::size_t x, std::size_t y) const
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    return data_[y * width() + x];
}



template<typename DataType>
void TableBase<DataType>::set(std::size_t x, std::size_t y, DataType data)
{
    if (x >= width() || y >= height()) {
        throw std::runtime_error("out of bounds");
    }
    modified_[y * width() + x] = true;
    data_[y * width() + x] = data;
}



template<typename DataType>
bool TableBase<DataType>::modified(std::size_t x, std::size_t y) const
{
    if (x >= width() || y >= height()) {
        return false;
    }
    return modified_[y * width() + x];
}



/*Table : public QAbstractTableModel {
    Q_OBJECT
public:
    virtual TableType type() const = 0;

    DataType dataType() const { return definition_->dataType(); }

    // Returns true if the table has been modified
    //from the base
    bool modified() const { return modified_; }

    void setModified(bool modified) { modified_ = modified; }

    ~Table() override = default;

    // Used to set the editable flag
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Serializes raw data. Returns false if the buffer is too small.
    virtual bool serialize(gsl::span<uint8_t> data,
                           Endianness endian = Endianness::Big) = 0;

    // Returns the minimum buffer size for serialization
    virtual size_t rawSize() = 0;

    // Sets modified data depending on the difference with another table
    virtual void calcDifference(const TablePtr &table) = 0;

    static std::shared_ptr<Table> create(TableType tableType, DataType dataType,
                                         const TableDefinition *def,
                                         Endianness endian,
                                         gsl::span<uint8_t> data);

protected:
    explicit Table(const TableDefinition *definition);

    template <typename T>
    static void readRow(std::vector<T> &data, Endianness endian,
                        gsl::span<const uint8_t> raw);

    template <typename T>
    static void writeRow(std::vector<T> &data, Endianness endian,
                         gsl::span<uint8_t> odata);

    template <typename T> static QString toString(T t);

    template <typename T>
    static T fromVariant(const QVariant &v, bool &success);

    bool modified_{};

    const TableDefinition *definition_;

signals:
    void onModified();
};
*/
    
/* One dimensional table */
/*

template <typename T>
Table2d<T>::Table2d(const TableDefinition *definition, Endianness endian,
                    gsl::span<const uint8_t> data)
    : Table(definition) {
    data_.resize(definition->sizeY());
    modifiedv_.resize(definition->sizeY(),
                      std::vector<bool>(definition->sizeX(), false));
    if (data.size() < sizeof(T) * definition->sizeX() * definition->sizeY()) {
        throw std::out_of_range(
            "data is too small to deserialize the table from");
    }
    const uint8_t *ptr = data.data();
    for (int i = 0; i < definition->sizeY(); ++i) {
        readRow(data_[i], endian,
                gsl::make_span(ptr, definition->sizeX() * sizeof(T)));
        ptr += sizeof(T) * definition->sizeX();
    }
}

template <typename T> void Table2d<T>::calcDifference(const TablePtr &table) {
    assert(table->type() == type());
    assert(table->dataType() == dataType());

    std::shared_ptr<Table2d> table2d = std::static_pointer_cast<Table2d>(table);

    for (int i = 0; i < definition_->sizeY(); ++i) {
        for (int x = 0; x < definition_->sizeX(); ++x) {
            if (data_[i][x] != table2d->at(x, i)) {
                modifiedv_[i][x] = true;
                modified_ = true;
            }
        }
    }
}

template <typename T>
bool Table2d<T>::serialize(gsl::span<uint8_t> data, Endianness endian) {
    if (data.size() < definition_->sizeX() * definition_->sizeY() * sizeof(T)) {
        return false;
    }
    for (std::vector<T> &row : data_) {
        writeRow(row, endian, data);
        data = data.subspan(definition_->sizeX() * sizeof(T));
    }

    return true;
}

template <typename T> size_t Table2d<T>::rawSize() {
    return definition_->sizeX() * definition_->sizeY() * sizeof(T);
}

template <typename T>
int Table2d<T>::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return definition_->sizeX();
}

template <typename T>
int Table2d<T>::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return definition_->sizeY();
}

template <typename T>
QVariant Table2d<T>::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= definition_->sizeY() ||
        index.column() < 0 || index.column() >= definition_->sizeX()) {
        return QVariant();
    }

    if (role == Qt::FontRole) {
        QFont font = QApplication::font("QTableView");
        if (modifiedv_[index.row()][index.column()]) {
            // Value has been modified
            font.setBold(true);
        } else {
            font.setBold(false);
        }
        return font;
    }

    if (role == Qt::BackgroundColorRole) {
        double ratio = static_cast<double>(at(index.column(), index.row()) -
                                           definition_->min()) /
                       (definition_->max() - definition_->min());
        return QColor::fromHsvF((1.0 - ratio) * (1.0 / 3.0), 1.0, 1.0);
    }
    if (role == Qt::ForegroundRole) {
        return QColor::fromRgb(0, 0, 0);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return toString(at(index.column(), index.row()));
}

template <typename T>
bool Table2d<T>::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    if (role != Qt::EditRole) {
        return false;
    }
    bool success;
    T res = fromVariant<T>(value, success);
    if (!success) {
        return false;
    }

    set(index.column(), index.row(), res);
    emit dataChanged(index, index);
    return true;
}

template <typename T>
Table1d<T>::Table1d(const TableDefinition *definition, Endianness endian,
                    gsl::span<const uint8_t> data)
    : Table(definition), modifiedv_(definition->sizeX()) {
    if (data.size() < sizeof(T) * definition->sizeX()) {
        throw std::out_of_range(
            "the size of data is too small to read the 1d table from");
    }
    readRow(data_, endian, data.subspan(0, definition->sizeX() * sizeof(T)));
}

template <typename T> void Table1d<T>::calcDifference(const TablePtr &table) {
    assert(table->type() == type());
    assert(table->dataType() == dataType());

    std::shared_ptr<Table1d> table1d = std::static_pointer_cast<Table1d>(table);

    for (int x = 0; x < definition_->sizeX(); ++x) {
        if (data_[x] != table1d->at(x)) {
            modifiedv_[x] = true;
            modified_ = true;
        }
    }
}

template <typename T>
bool Table1d<T>::serialize(gsl::span<uint8_t> data, Endianness endian) {
    if (data.size() < data_.size() * sizeof(T)) {
        return false;
    }
    writeRow(data_, endian, data);

    return true;
}

template <typename T> size_t Table1d<T>::rawSize() {
    return data_.size() * sizeof(T);
}

template <typename T>
int Table1d<T>::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return definition_->sizeX();
}

template <typename T>
QVariant Table1d<T>::data(const QModelIndex &index, int role) const {
    if (index.row() != 0 || index.column() < 0 ||
        index.column() >= definition_->sizeX()) {
        return QVariant();
    }

    if (role == Qt::FontRole) {
        QFont font = QApplication::font("QTableView");
        if (modifiedv_[index.column()]) {
            // Value has been modified
            font.setWeight(QFont::Medium);
        } else {
            font.setWeight(QFont::Normal);
        }
        return font;
    }
    if (role == Qt::ForegroundRole) {
        if (data_.size() == 1) {
            // TODO: replace this with a check of the background color
            return QVariant();
        }
        return QColor::fromRgb(0, 0, 0);
    }

    if (role == Qt::BackgroundColorRole) {
        double ratio =
            static_cast<double>(at(index.column()) - definition_->min()) /
            (definition_->max() - definition_->min());
        if (ratio < 0.0) {
            return QVariant();
        }
        return QColor::fromHsvF((1.0 - ratio) * (1.0 / 3.0), 1.0, 1.0);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return toString(at(index.column()));
}

template <typename T>
int Table1d<T>::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return 1;
}

template <typename T>
bool Table1d<T>::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    if (role != Qt::EditRole) {
        return false;
    }
    bool success;
    T res = fromVariant<T>(value, success);
    if (!success) {
        return false;
    }

    set(index.column(), res);
    emit dataChanged(index, index);
    return true;
}


template <typename T>
void Table::readRow(std::vector<T> &data, Endianness endian,
                    gsl::span<const uint8_t> raw) {
    const uint8_t *ptr = raw.data();
    const uint8_t *end = raw.data() + raw.size();
    if (endian == Endianness::Big) {
        for (; ptr < end; ptr += sizeof(T)) {
            data.push_back(readBE<T>(gsl::make_span(ptr, end)));
        }
        return;
    }

    // Little endian
    for (; ptr < end; ptr += sizeof(T)) {
        data.push_back(readLE<T>(gsl::make_span(ptr, end)));
    }
}

template <typename T>
void Table::writeRow(std::vector<T> &data, Endianness endian,
                     gsl::span<uint8_t> odata) {
    if (endian == Endianness::Big) {
        for (T f : data) {
            writeBE<T>(f, odata);
            odata = odata.subspan(sizeof(T));
        }
        return;
    }

    // Little endian
    for (float f : data) {
        writeLE<T>(f, odata);
        odata = odata.subspan(sizeof(T));
    }
}

template <typename T> QString Table::toString(T t) {
    return QString::number(t);
}

template <> inline QString Table::toString<float>(float t) {
    return QString::number(t, 'f', 2);
}

template <typename T> T Table::fromVariant(const QVariant &v, bool &success) {
    if (v.canConvert<T>()) {
        success = true;
        return v.value<T>();
    }
    success = false;
    return T();
}

template <>
inline float Table::fromVariant<float>(const QVariant &v, bool &success) {
    return v.toFloat(&success);
}*/


#endif // TABLE_H
