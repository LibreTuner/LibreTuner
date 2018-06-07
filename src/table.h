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

#include "definitions/tabledefinitions.h"
#include "endian.h"
#include "util.hpp"

#include <cassert>
#include <memory>
#include <gsl/span>

#include <QAbstractTableModel>
#include <QApplication>
#include <QColor>
#include <QFont>

class Table;
typedef std::shared_ptr<Table> TablePtr;

/**
 * @todo write docs
 */
class Table : public QAbstractTableModel {
  Q_OBJECT
public:
  virtual TableType type() const = 0;

  DataType dataType() const { return definition_->dataType(); }

  const TableDefinition *definition() const { return definition_; }

  /* Returns true if the table has been modified
   * from the base */
  bool modified() const { return modified_; }

  void setModified(bool modified) { modified_ = modified; }

  ~Table() override = default;

  /* Used to set the editable flag */
  virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  /* Serializes raw data. Returns false if the buffer is too small. */
  virtual bool serialize(gsl::span<uint8_t> data,
                         Endianness endian = ENDIAN_BIG) = 0;

  /* Returns the minimum buffer size for serialization */
  virtual size_t rawSize() = 0;

  /* Sets modified data depending on the difference with another table */
  virtual void calcDifference(const TablePtr &table) = 0;

  static std::shared_ptr<Table> create(TableType tableType, DataType dataType, const TableDefinition *def, Endianness endian, gsl::span<uint8_t> data);

protected:
  explicit Table(const TableDefinition *definition);

  template <typename T>
  static void readRow(std::vector<T> &data, Endianness endian,
                      gsl::span<const uint8_t> raw);

  template <typename T>
  static void writeRow(std::vector<T> &data, Endianness endian, gsl::span<uint8_t> odata);

  template <typename T> static QString toString(T t);

  template <typename T> static T fromVariant(const QVariant &v, bool &success);

  bool modified_{};

  const TableDefinition *definition_;

signals:
  void onModified();
};

/* One dimensional table */
template <typename T> class Table1d : public Table {
public:
  /* Constructs a new table using an array of T */
  /*Table1d(const TableDefinition *definition, const T *data, size_t length)
      : data_(data, data + length), modifiedv_(definition->sizeX()) {}*/

  /* Constructs a new table using raw data.
   * data should be at least (sizeof(T) * length) bytes large */
  Table1d(const TableDefinition *definition, Endianness endian,
          gsl::span<const uint8_t> data);

  T at(int idx) const { return data_[idx]; }

  void set(int idx, T t) {
    if (at(idx) == t) {
      return;
    }
    data_[idx] = t;
    modifiedv_[idx] = true;
    modified_ = true;
    emit onModified();
  }

  T &operator[](int idx) { return data_[idx]; }

  TableType type() const override { return TABLE_1D; }

  void calcDifference(const TablePtr &table) override;

  bool serialize(gsl::span<uint8_t> data,
                 Endianness endian = ENDIAN_BIG) override;

  size_t rawSize() override;

  /* Qt model functions */
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  int rowCount(const QModelIndex &parent) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

private:
  std::vector<T> data_;
  /* Tracks modified values. */
  std::vector<bool> modifiedv_;
};

/* Two dimensional table */
template <typename T> class Table2d : public Table {
public:
  /* Constructs a new table using an array of T */
  /*Table2d(const TableDefinition *definition, const T **data, size_t width,
  size_t height) : data_(data, data + length)
  {
  }*/

  /* Constructs a new table using raw data.
   * data should be at least (sizeof(T) * sizeX * sizeY) bytes large */
  Table2d(const TableDefinition *definition, Endianness endian,
          gsl::span<const uint8_t> data);

  T at(int x, int y) const { return data_[y][x]; }

  void set(int x, int y, T t) {
    if (at(x, y) == t) {
      return;
    }
    data_[y][x] = t;
    modifiedv_[y][x] = true;
    modified_ = true;
    emit onModified();
  }

  TableType type() const override { return TABLE_2D; }

  void calcDifference(const TablePtr &table) override;

  bool serialize(gsl::span<uint8_t> data,
                 Endianness endian = ENDIAN_BIG) override;

  size_t rawSize() override;

  /* Qt model functions */
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  int rowCount(const QModelIndex &parent) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

private:
  std::vector<std::vector<T>> data_;
  /* Tracks modified values. */
  std::vector<std::vector<bool>> modifiedv_;
};

template <typename T>
Table2d<T>::Table2d(const TableDefinition *definition, Endianness endian,
                    gsl::span<const uint8_t> data)
    : Table(definition) {
  data_.resize(definition->sizeY());
  modifiedv_.resize(definition->sizeY(),
                    std::vector<bool>(definition->sizeX(), false));
  if (data.size() < sizeof(T) * definition->sizeX() * definition->sizeY()) {
    throw std::out_of_range("data is too small to deserialize the table from");
  }
  const uint8_t *ptr = data.data();
  for (int i = 0; i < definition->sizeY(); ++i) {
    readRow(data_[i], endian, gsl::make_span(ptr, definition->sizeX() * sizeof(T)));
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
    throw std::out_of_range("the size of data is too small to read the 1d table from");
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


/* Table */
template <typename T>
void Table::readRow(std::vector<T> &data, Endianness endian,
                    gsl::span<const uint8_t> raw) {
  const uint8_t *ptr = raw.data();
  const uint8_t *end = raw.data() + raw.size();
  if (endian == ENDIAN_BIG) {
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
  if (endian == ENDIAN_BIG) {
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

template <typename T>
QString Table::toString(T t) {
  return QString::number(t);
}

template <typename T>
T Table::fromVariant(const QVariant &v, bool &success) {
  if (v.canConvert<T>()) {
    success = true;
    return v.value<T>();
  }
  success = false;
  return T();
}

#endif // TABLE_H
