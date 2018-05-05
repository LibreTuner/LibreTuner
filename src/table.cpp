#include "table.h"
#include "util.hpp"

Table::Table(const TableDefinition* definition) : definition_(definition)
{
}



Qt::ItemFlags Table::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}



QVariant Table::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || section < 0)
    {
        return QVariant();
    }
    
    
    if (orientation == Qt::Horizontal)
    {
        if (definition_->axisX() != nullptr && section < definition_->sizeX())
        {
            return QString::number(definition_->axisX()->label(section));
        }
    }
    else if (orientation == Qt::Vertical)
    {
        if (definition_->axisY() != nullptr && section < definition_->sizeY())
        {
            return QString::number(definition_->axisY()->label(section));
        }
    }
    
    return QVariant();
}



template<>
QString Table::toString<float>(float t)
{
    return QString::number(t, 'f', 2);
}



template<>
float Table::fromVariant<float>(const QVariant &v, bool &success)
{
    return v.toFloat(&success);
}




template<>
void Table::readRow<float>(std::vector<float> &data, Endianness endian, const uint8_t *raw, size_t length)
{
    if (endian == ENDIAN_BIG)
    {
        for (int i = 0; i < length; ++i)
        {
            data.push_back(toBEFloat(raw));
            raw += sizeof(float);
        }
        return;
    }

    // Little endian
    for (int i = 0; i < length; ++i)
    {
        data.push_back(toLEFloat(raw));
        raw += sizeof(float);
    }
}



template<>
void Table::writeRow<float>(std::vector<float> &data, Endianness endian, uint8_t *odata)
{
    if (endian == ENDIAN_BIG)
    {
        for (float f : data)
        {
            uint32_t raw = *reinterpret_cast<uint32_t*>(&f);
            *(odata++) = (raw >> 24);
            *(odata++) = ((raw >> 16) & 0xFF);
            *(odata++) = ((raw >> 8) & 0xFF);
            *(odata++) = (raw & 0xFF);
        }
        return;
    }
    
    // Little endian
    for (float f : data)
    {
        uint32_t raw = *reinterpret_cast<uint32_t*>(&f);
        *(odata++) = (raw & 0xFF);
        *(odata++) = ((raw >> 8) & 0xFF);
        *(odata++) = ((raw >> 16) & 0xFF);
        *(odata++) = (raw >> 24);
    }
}






