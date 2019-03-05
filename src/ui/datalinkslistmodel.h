#ifndef DATALINKSLISTMODEL_H
#define DATALINKSLISTMODEL_H

#include <QAbstractItemModel>

namespace datalink {
    class LinkDatabase;
};

/**
 * List of all datalinks. Combines autodetected and manual
 */
class DataLinksListModel : public QAbstractListModel
{
public:
    virtual QVariant data(const QModelIndex& index, int role) const override;

    virtual int columnCount(const QModelIndex& parent) const override;

    virtual int rowCount(const QModelIndex& parent) const override;

    virtual QModelIndex parent(const QModelIndex& child) const override;

    void setDatabase(datalink::LinkDatabase *db) { links_ = db; }
    
private:
    datalink::LinkDatabase *links_{nullptr};
};

#endif // DATALINKSLISTMODEL_H
