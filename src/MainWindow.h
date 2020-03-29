#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QPointer>

#include <models/CategorizedTablesModel.h>
#include <models/TableDetailsModel.h>
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

    void displayDownloadDialog();

    // Tries to close existing calibration. Returns false if the user chooses to keep it open.
    bool closeCalibration();

    // Returns true if the calibration was saved.
    bool saveCalibration(bool changePath = false);

    void setDirty();
    void clearDirty();

    // Returns false if the calibration cannot be set. (e.g.
    bool setCalibration(const lt::Platform * platform, const uint8_t * data, std::size_t size,
                        const QString & path = QString());

protected:
    void closeEvent(QCloseEvent * event) override;

private slots:
    void on_actionOpen_triggered();

    void on_treeView_activated(const QModelIndex & index);

    void on_tabs_tabCloseRequested(int index);

    void on_tabs_currentChanged(int index);

    void on_actionDownload_triggered();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

private:
    Ui::MainWindow * ui;

    CategorizedTablesModel tablesModel_;
    QSortFilterProxyModel tablesSortModel_;
    TableDetailsModel detailsModel_;

    lt::Calibration calibration_;

    QString calibrationPath_;
    QString calibrationName_;

    bool dirty_{false};

    std::unordered_map<std::string, QPointer<QWidget>> openedTables_;
};

#endif // EDITOR_H
