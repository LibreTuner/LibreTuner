#ifndef TABLESWIDGET_H
#define TABLESWIDGET_H

#include <QWidget>

class QTreeView;
class QAbstractItemModel;

class TablesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TablesWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

signals:

public slots:

private:
    QTreeView *view_;
};

#endif // TABLESWIDGET_H
