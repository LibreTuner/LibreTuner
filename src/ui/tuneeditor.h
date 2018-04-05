#ifndef TUNEEDITOR_H
#define TUNEEDITOR_H

#include <QMainWindow>
#include <QAbstractItemModel>
#include <QTreeWidgetItem>

#include <memory>

namespace Ui
{
class TuneEditor;
}


class TuneData;
typedef std::shared_ptr<TuneData> TuneDataPtr;
class Table;
typedef std::shared_ptr<Table> TablePtr;

/**
 * Tune editor window
 */
class TuneEditor : public QMainWindow
{
    Q_OBJECT
public:
    TuneEditor(TuneDataPtr tune, QWidget *parent = 0);

private:
    TuneDataPtr tune_;
    Ui::TuneEditor* ui;
    TablePtr currentTable_;
    
private slots:
    void on_treeTables_itemActivated(QTreeWidgetItem *item, int column);
};

#endif // TUNEEDITOR_H
