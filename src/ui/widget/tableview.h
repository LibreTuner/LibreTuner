#ifndef LIBRETUNER_TABLEVIEW_H
#define LIBRETUNER_TABLEVIEW_H

#include <QWidget>

#include "../verticallabel.h"
#include "models/TableModel.h"

class QTableView;
class QLabel;
class GraphWidget;
class QBoxLayout;

class TableView : public QWidget
{
public:
    explicit TableView(QWidget * parent = nullptr);
    ~TableView() override;

    void setTable(lt::Table * table);

private slots:
    void axesChanged();

private:
    QTableView * view_;
    QLabel * labelX_;
    VerticalLabel * labelY_;
    GraphWidget * graph_;
    QBoxLayout * layout_;

    TableModel model_;
};

#endif // LIBRETUNER_TABLEVIEW_H
