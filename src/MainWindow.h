#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QPointer>

#include <models/CategorizedTablesModel.h>
#include <rom/rom.h>

class GraphWidget;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = nullptr);
    ~MainWindow();

    void displayQuickStartDialog();

    void importCalibration(const QString & path);

protected:
    void closeEvent(QCloseEvent * event) override;

private slots:
    void on_actionOpen_triggered();

    void on_treeView_activated(const QModelIndex & index);

    void on_tabs_tabCloseRequested(int index);

    void on_tabs_currentChanged(int index);

private:
    Ui::MainWindow * ui;

    CategorizedTablesModel tablesModel_;
    QSortFilterProxyModel tablesSortModel_;

    lt::Calibration calibration_;

    std::unordered_map<std::string, QPointer<QWidget>> openedTables_;
};

#endif // EDITOR_H
