#ifndef LIBRETUNER_EXPLORERWIDGET_H
#define LIBRETUNER_EXPLORERWIDGET_H

#include <QWidget>

#include <lt/project/project.h>

class QTreeView;
class QAbstractItemModel;

class ExplorerWidget : public QWidget
{
public:
    explicit ExplorerWidget(QWidget * parent = nullptr);

    void setModel(QAbstractItemModel * model);

private:
    QTreeView * tree_;
};

#endif // LIBRETUNER_EXPLORERWIDGET_H
