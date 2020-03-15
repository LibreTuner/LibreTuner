#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTableView>

#include "dialogs/ImportCalibrationDialog.h"
#include "dialogs/QuickStartDialog.h"

#include <models/TableModel.h>
#include <rom/rom.h>
#include <widgets/TableView.h>

MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    tablesSortModel_.setSourceModel(&tablesModel_);
    ui->treeView->setModel(&tablesSortModel_);

    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreState(settings.value("State").toByteArray());
    restoreGeometry(settings.value("Geometry").toByteArray());
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::displayQuickStartDialog()
{
    QuickStartDialog qs;
    if (qs.exec() != QDialog::Accepted)
        return;

    importCalibration(qs.selectedCalibration());
}

void MainWindow::importCalibration(const QString & path)
{
    ImportCalibrationDialog c(path);
    if (c.exec() != QDialog::Accepted)
        return;

    const lt::Platform * platform = c.selectedPlatform();
    if (platform == nullptr)
        return;

    // Load file
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Import Error", "Could not open file '" + path + "'");
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    const lt::Model * model = platform->identify(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    if (model == nullptr)
    {
        QMessageBox::warning(this, "Import Error", "Could not find a definition for the calibration");
        return;
    }

    // TODO: Close existing calibration
    calibration_.setModel(model);
    calibration_.setData(lt::MemoryBuffer(data.begin(), data.end()));

    tablesModel_.setDefinition(model);

    // Add path to history
    QSettings settings;
    settings.beginGroup("QuickStart");
    QStringList history = settings.value("History").toStringList();
    if (!history.contains(path))
    {
        history.push_front(path);
        settings.setValue("History", history);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open calibration file"), QString(),
                                                "Calibration (*.rom *.bin);;All files (*)");
    if (!path.isEmpty())
        importCalibration(path);
}

void MainWindow::on_treeView_activated(const QModelIndex & index)
{
    const auto * ti = tablesSortModel_.data(index, Qt::UserRole).value<const lt::TableDefinition *>();
    if (ti == nullptr)
        return;

    if (auto it = openedTables_.find(ti->id); it != openedTables_.end() && it->second)
        return;

    if (auto table = calibration_.getTable(ti->id))
    {
        QPointer<TableView> view = new TableView(std::move(*table));
        view->setAttribute(Qt::WA_DeleteOnClose);
        ui->tabs->setCurrentIndex(ui->tabs->addTab(view, QString::fromStdString(ti->name)));

        openedTables_.insert_or_assign(ti->id, std::move(view));
    }
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    delete ui->tabs->widget(index);
}

void MainWindow::on_tabs_currentChanged(int index)
{
    auto * tab = reinterpret_cast<TableView*>(ui->tabs->widget(index));
    ui->graph->setModel(tab->model());
}

void MainWindow::closeEvent(QCloseEvent * event) {
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("State", saveState());
    settings.setValue("Geometry", saveGeometry());

    QWidget::closeEvent(event);
}
