#ifndef TABLE_H
#define TABLE_H

/* Because this file includes function definitions, only include
 * it if absolutely necessary to decrease compile times. */

#include "tabledefinitions.h"

#include <memory>

#include <QAbstractTableModel>
#include <QApplication>
#include <QFont>
#include <QColor>

/**
 * @todo write docs
 */
class Table : public QAbstractTableModel
{
public:
    virtual TableType type() const =0;

    DataType dataType() const
    {
        return definition_->dataType();
    }
    
    const TableDefinition *definition() const
    {
        return definition_;
    }
    
    /* Returns true if the table has been modified
     * from the base */
    bool modified() const
    {
        return modified_;
    }

    virtual ~Table() {};
    
    /* Used to set the editable flag */
    virtual Qt::ItemFlags flags(const QModelIndex & index) const override;
    
protected:
    Table(const TableDefinition *definition);
    
    template<typename T>
    static void readRow(std::vector<T> &data, Endianness endian, const uint8_t *raw, size_t length);
    
    template<typename T>
    static QString toString(T t);
    
    template<typename T>
    static T fromVariant(const QVariant &v, bool &success);
    
    bool modified_;
    
    const TableDefinition *definition_;
};
typedef std::shared_ptr<Table> TablePtr;



/* One dimensional table */
template<typename T>
class Table1d : public Table
{
public:
    /* Constructs a new table using an array of T */
    Table1d(const TableDefinition *definition, const T *data, size_t length) : data_(data, data + length), modifiedv_(definition->sizeX())
    {}
    
    /* Constructs a new table using raw data. 
     * data should be at least (sizeof(T) * length) bytes large */
    Table1d(const TableDefinition *definition, Endianness endian, const uint8_t *data);
    
    T at(int idx) const
    {
        return data_[idx];
    }
    
    void set(int idx, T t)
    {
        if (at(idx) == t)
        {
            return;
        }
        data_[idx] = t;
        modifiedv_[idx] = true;
        modified_ = true;
    }
    
    T &operator[](int idx)
    {
        return data_[idx];
    }
    
    TableType type() const override
    {
        return TABLE_1D;
    }
    
    /* Qt model functions */
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    int rowCount(const QModelIndex & parent) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    
private:
    std::vector<T> data_;
    /* Tracks modified values. */
    std::vector<bool> modifiedv_;
};




/* Two dimensional table */
template<typename T>
class Table2d : public Table
{
public:
    /* Constructs a new table using an array of T */
    /*Table2d(const TableDefinition *definition, const T **data, size_t width, size_t height) : data_(data, data + length)
    {
    }*/
    
    /* Constructs a new table using raw data.
     * data should be at least (sizeof(T) * sizeX * sizeY) bytes large */
    Table2d(const TableDefinition *definition, Endianness endian, const uint8_t *data);
    
    T at(int x, int y) const
    {
        return data_[y][x];
    }
    
    void set(int x, int y, T t)
    {
        if (at(x, y) == t)
        {
            return;
        }
        data_[y][x] = t;
        modifiedv_[y][x] = true;
        modified_ = true;
    }
    
    TableType type() const override
    {
        return TABLE_2D;
    }
    
    /* Qt model functions */
    int columnCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    int rowCount(const QModelIndex & parent) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;
    
private:
    std::vector<std::vector<T> > data_;
    /* Tracks modified values. */
    std::vector<std::vector<bool> > modifiedv_;
};










template<typename T>
Table2d<T>::Table2d(const TableDefinition* definition, Endianness endian, const uint8_t* data) : Table(definition)
{
    data_.resize(definition->sizeY());
    modifiedv_.resize(definition->sizeY(), std::vector<bool>(definition->sizeX(), false));
    for (int i = 0; i < definition->sizeY(); ++i)
    {
        readRow(data_[i], endian, data, definition->sizeX());
        data += sizeof(T) * definition->sizeX();
    }
}



template<typename T> 
int Table2d<T>::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return definition_->sizeX();
}



template<typename T>
int Table2d<T>::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return definition_->sizeY();
}



template<typename T>
QVariant Table2d<T>::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= definition_->sizeY() ||
        index.column() < 0 || index.column() >= definition_->sizeX())
    {
        return QVariant();
    }
    
    if (role == Qt::FontRole)
    {
        QFont font = QApplication::font("QTableView");
        if (modifiedv_[index.row()][index.column()])
        {
            // Value has been modified
            font.setBold(true);
        }
        else
        {
            font.setBold(false);
        }
        return font;
    }
    
    if (role == Qt::BackgroundColorRole)
    {
        if (index.row() % 2 == 0)
        {
            // Even row
            return QColor::fromRgb(255, 140, 140);
        }
        return QColor::fromRgb(255, 166, 166);
    }
    
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return QVariant();
    }
    
    return toString(at(index.column(), index.row()));
}



template<typename T>
bool Table2d<T>::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }
    bool success;
    T res = fromVariant<T>(value, success);
    if (!success)
    {
        return false;
    }
    
    set(index.column(), index.row(), res);
    return true;
}



template<typename T>
Table1d<T>::Table1d(const TableDefinition *definition, Endianness endian, const uint8_t *data) : Table(definition), modifiedv_(definition->sizeX())
{
    readRow(data_, endian, data, definition->sizeX());
}



template<typename T>
int Table1d<T>::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return definition_->sizeX();
}



template<typename T>
QVariant Table1d<T>::data(const QModelIndex& index, int role) const
{
    if (index.row() != 0 || index.column() < 0 || index.column() >= definition_->sizeX())
    {
        return QVariant();
    }
    
    if (role == Qt::FontRole)
    {
        QFont font = QApplication::font("QTableView");
        if (modifiedv_[index.column()])
        {
            // Value has been modified
            font.setWeight(QFont::Medium);
        }
        else
        {
            font.setWeight(QFont::Normal);
        }
        return font;
    }
    
    if (role == Qt::BackgroundColorRole)
    {
        return QColor::fromRgb(255, 140, 140);
    }
    
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return QVariant();
    }
    
    return toString(at(index.column()));
}



template<typename T>
int Table1d<T>::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return 1;
}



template<typename T>
bool Table1d<T>::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }
    bool success;
    T res = fromVariant<T>(value, success);
    if (!success)
    {
        return false;
    }
    
    set(index.column(), res);
    return true;
}


#endif // TABLE_H
