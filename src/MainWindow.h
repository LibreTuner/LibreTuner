#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QSortFilterProxyModel>

#include <models/CategorizedTablesModel.h>
#include <rom/rom.h>

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

private slots:
    void on_actionOpen_triggered();

    void on_treeView_activated(const QModelIndex & index);

private:
    Ui::MainWindow * ui;

    CategorizedTablesModel tablesModel_;
    QSortFilterProxyModel tablesSortModel_;

    lt::Calibration calibration_;
};

#endif // EDITOR_H
