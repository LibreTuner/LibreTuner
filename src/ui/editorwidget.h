#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

class QTableView;
class QAbstractItemModel;
class QLabel;
class VerticalLabel;

class EditorWidget : public QWidget
{
public:
    explicit EditorWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

private:
    QTableView *view_;
    QLabel *labelX_;
    VerticalLabel *labelY_;
};

#endif // EDITORWIDGET_H
