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

MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    tablesSortModel_.setSourceModel(&tablesModel_);
    ui->treeView->setModel(&tablesSortModel_);
}

MainWindow::~MainWindow() { delete ui; }

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
    settings.beginGroup("Quick Start");
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

    if (auto table = calibration_.getTable(ti->id))
    {
        auto * model = new TableModel(std::move(*table));

        auto * view = new QTableView(this);
        view->setModel(model);
        view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tabs->addTab(view, QString::fromStdString(ti->name));
    }
}
