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

    void closeEvent(QCloseEvent * event) override;
    
    bool save();
private:
    TuneDataPtr tune_;
    Ui::TuneEditor* ui;
    TablePtr currentTable_;
    
    /* Set to true if a table has been modified */
    bool changed_ = false;
    
private slots:
    void on_treeTables_itemActivated(QTreeWidgetItem *item, int column);
    void onTableModified();
    
signals:
    void tableChanged(TablePtr table);
};

#endif // TUNEEDITOR_H
