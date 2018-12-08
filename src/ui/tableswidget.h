#ifndef TABLESWIDGET_H
#define TABLESWIDGET_H

#include <QWidget>

#include <memory>
#include <vector>

#include "definitions/definition.h"

class Tune;

class QTreeWidget;
class QAbstractItemModel;

class TablesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TablesWidget(QWidget *parent = nullptr);

    //void setModel(QAbstractItemModel *model);

signals:
    void activated(int index);

public slots:
    void setTables(const std::vector<definition::Table> &tables);

private:
    QTreeWidget *view_;
};

#endif // TABLESWIDGET_H
