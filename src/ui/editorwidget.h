#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QWidget>

#include <memory>

class QTableView;
class QAbstractItemModel;
class QLabel;
class VerticalLabel;

class Tune;
class TuneData;
class Table;

class EditorWidget : public QWidget
{
public:
    explicit EditorWidget(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);
    
public slots:
    void tableChanged(Table *table);
    void tuneChanged(const std::shared_ptr<Tune> &tune);

private:
    QTableView *view_;
    QLabel *labelX_;
    VerticalLabel *labelY_;
    std::shared_ptr<Tune> tune_;
    std::shared_ptr<TuneData> tuneData_;
};

#endif // EDITORWIDGET_H
