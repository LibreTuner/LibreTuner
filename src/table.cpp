#include "table.h"
#include "util.hpp"

Table::Table(const TableDefinition* definition) : definition_(definition)
{
}



Qt::ItemFlags Table::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
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
