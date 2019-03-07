//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include <QWidget>
#include <QAbstractItemModel>
#include "datalink/linkdatabase.h"

class DataLinksTreeModel : public QAbstractItemModel {
public:
    int columnCount(const QModelIndex & parent) const override;
    
    QVariant data(const QModelIndex & index, int role) const override;
    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    
    QModelIndex index(int row, int column, const QModelIndex & parent) const override;
    
    QModelIndex parent(const QModelIndex & child) const override;
    
    int rowCount(const QModelIndex & parent) const override;
    
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    
    void setLinks(datalink::LinkDatabase *links);
    
private:
    datalink::LinkDatabase *links_{nullptr};
    datalink::LinkDatabase::UpdateConn updateConnection_;
};


class DatalinksWidget : public QWidget {
public:
    explicit DatalinksWidget(QWidget *parent = nullptr);
    
private:
    DataLinksTreeModel linksModel_;
};


#endif //LIBRETUNER_INTERFACESDIALOG_H
