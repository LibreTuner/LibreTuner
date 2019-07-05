#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

#include <memory>

class QTableView;
class QAbstractItemModel;
class QLabel;
class VerticalLabel;

class TableModel;

class EditorWidget : public QWidget
{
public:
    explicit EditorWidget(QWidget * parent = nullptr);

    void setModel(TableModel * model);

private slots:
    void axesChanged();

private:
    QTableView * view_;
    QLabel * labelX_;
    VerticalLabel * labelY_;

    TableModel * model_{nullptr};
};

#endif // EDITORWIDGET_H
