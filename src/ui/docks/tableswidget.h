#ifndef TABLESWIDGET_H
#define TABLESWIDGET_H

#include <QWidget>

#include <memory>
#include <vector>

#include "lt/definition/platform.h"

class QTreeWidget;
class QAbstractItemModel;

class TablesWidget : public QWidget {
    Q_OBJECT
public:
    explicit TablesWidget(QWidget *parent = nullptr);

signals:
    void activated(const lt::TableDefinition *table);

public slots:
    void setModel(const lt::Model &model);

private:
    QTreeWidget *view_;
};

Q_DECLARE_METATYPE(const lt::TableDefinition *)

#endif // TABLESWIDGET_H
