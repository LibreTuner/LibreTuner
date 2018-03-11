#ifndef ROMLISTMODEL_H
#define ROMLISTMODEL_H

#include <QAbstractItemModel>

/**
 * @todo write docs
 */
class RomListModel : public QAbstractListModel
{
public:

    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual int columnCount(const QModelIndex& parent) const;

    virtual int rowCount(const QModelIndex& parent) const;

};

#endif // ROMLISTMODEL_H
