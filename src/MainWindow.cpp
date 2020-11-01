#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTableView>
#include <QChartView>

#include "BackgroundTask.h"
#include "dialogs/DownloadDialog.h"
#include "dialogs/ImportCalibrationDialog.h"
#include "dialogs/QuickStartDialog.h"
#include "uiutil.h"

#include <dialogs/FlashDialog.h>
#include <link/platformlink.h>
#include <models/TableModel.h>
#include <rom/rom.h>
#include <widgets/TableView.h>

MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    tablesSortModel_.setSourceModel(&tablesModel_);
    tablesSortModel_.setFilterKeyColumn(0);
    tablesSortModel_.setFilterRole(Qt::DisplayRole);
    tablesSortModel_.setRecursiveFilteringEnabled(true);
    ui->treeView->setModel(&tablesSortModel_);
    ui->treeDetails->setModel(&detailsModel_);

    pidsSortModel_.setSourceModel(&pidsModel_);
    pidsSortModel_.setFilterKeyColumn(0);
    pidsSortModel_.setFilterRole(Qt::DisplayRole);
    pidsSortModel_.setRecursiveFilteringEnabled(true);
    ui->treePids->setModel(&pidsSortModel_);

    // Create datalog chart
    auto * chart = new QChart;
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setBackgroundRoundness(0);
    chart->layout()->setContentsMargins(0, 0, 0, 0);

    datalogChartView_ = new QtCharts::QChartView(chart);
    datalogChartView_->setVisible(false);

    datalogHelper_ = new QLabel(tr("Use the 'PID' menu to select PIDs and start logging"));
    datalogHelper_->setAlignment(Qt::AlignCenter);

    auto *datalogLayout = new QVBoxLayout;
    datalogLayout->addWidget(datalogChartView_);
    datalogLayout->addWidget(datalogHelper_);
    ui->dockDatalogContents->setLayout(datalogLayout);

    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreState(settings.value("State").toByteArray());
    restoreGeometry(settings.value("Geometry").toByteArray());

    updateRecent();

    connect(ui->lineSearch, &QLineEdit::textEdited, [this](const QString &) { updateSearch(); });
    connect(ui->checkRegex, &QCheckBox::stateChanged, [this](int) { updateSearch(); });
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::displayQuickStartDialog()
{
    QuickStartDialog qs;
    if (qs.exec() != QDialog::Accepted)
        return;

    switch (qs.mode())
    {
    case QuickStartMode::Open:
        importCalibration(qs.selectedCalibration());
        break;
    case QuickStartMode::Download:
        displayDownloadDialog();
        break;
    }
}

void MainWindow::importCalibration(const QString & path)
{
    if (!closeCalibration())
        return;

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

    if (!setCalibration(platform, reinterpret_cast<const uint8_t *>(data.data()), data.size(), path))
        QMessageBox::warning(this, "Import Error", "Could not find a definition for the calibration");

    // Add path to history
    {
        QSettings settings;
        settings.beginGroup("QuickStart");
        QStringList history = settings.value("History").toStringList();
        history.removeAll(path);

        history.push_front(path);
        settings.setValue("History", history);
    }

    updateRecent();
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
    {
        if (it->second)
            ui->tabs->setCurrentWidget(it->second);
        return;
    }

    if (auto table = calibration_.getTable(ti->id))
    {
        QPointer<TableView> view = new TableView(std::move(*table));
        view->setAttribute(Qt::WA_DeleteOnClose);
        ui->tabs->setCurrentIndex(ui->tabs->addTab(view, QString::fromStdString(ti->name)));

        connect(view->model(), &TableModel::dataChanged,
                [this](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles) {
                    setDirty();
                });

        openedTables_.insert_or_assign(ti->id, std::move(view));
    }
}

void MainWindow::on_tabs_tabCloseRequested(int index) { delete ui->tabs->widget(index); }

void MainWindow::on_tabs_currentChanged(int index)
{
    auto * tab = reinterpret_cast<TableView *>(ui->tabs->widget(index));
    if (tab == nullptr)
    {
        ui->graph->setModel(nullptr);
        detailsModel_.setTable(nullptr);
        return;
    }

    ui->graph->setModel(tab->model());
    detailsModel_.setTable(&tab->model()->table());
    ui->labelDescription->setText(QString::fromStdString(tab->model()->table().description()));
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("State", saveState());
    settings.setValue("Geometry", saveGeometry());

    if (!closeCalibration())
    {
        event->ignore();
        return;
    }

    QWidget::closeEvent(event);
}

void MainWindow::displayDownloadDialog()
{
    if (!closeCalibration())
        return;

    DownloadDialog dd;
    if (dd.exec() != QDialog::Accepted)
        return;

    lt::DataLink * link = dd.dataLink();
    if (link == nullptr)
    {
        QMessageBox::warning(this, tr("Unable to download"), tr("An interface has not been selected"));
        return;
    }

    const lt::Platform * platform = dd.platform();
    if (platform == nullptr)
    {
        QMessageBox::warning(this, tr("Unable to download"), tr("A platform has not been selected"));
        return;
    }

    lt::PlatformLink pl(*link, *platform);
    lt::download::DownloaderPtr downloader;
    catchWarning([&pl, &downloader]() { downloader = pl.downloader(); }, tr("Unable to download"));

    if (!downloader)
    {
        QMessageBox::warning(this, tr("Unable to download"),
                             tr("Unable to create a downloader. Does the platform support downloading and is the "
                                "proper interface selected?"));
        return;
    }

    QProgressDialog progress(tr("Download Calibration"), tr("Cancel"), 0, 10000, this);
    progress.show();
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(false);

    downloader->setProgressCallback([&progress](float v) {
        QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection,
                                  Q_ARG(int, static_cast<int>(v * 10000.f)));
    });

    connect(&progress, &QProgressDialog::canceled, [&downloader]() { downloader->cancel(); });

    std::atomic_bool successful(false);
    BackgroundTask<void()> bt([&downloader, &successful]() { successful = downloader->download(); });
    catchCritical(
        [&bt]() {
            bt();
            bt.future().get();
        },
        tr("Error while downloading"));

    if (!successful)
        return;

    auto [data, size] = downloader->data();

    // Save backup somewhere

    if (!setCalibration(platform, data, size))
    {
        QMessageBox::information(
            this, tr("Unknown model"),
            tr("The calibration was successfully downloaded but there are no definitions for this model. Please save "
               "and send the calibration to the Overboost developers at support@libretuner.org."));
    }
    else
    {
        int res =
            QMessageBox::question(this, tr("Save stock calibration"),
                                  tr("The download was successful. It is recommended that you backup this calibration "
                                     "somewhere safe. IF YOU MODIFY THIS CALIBRATION WITHOUT BACKING IT UP, YOU WILL "
                                     "NOT BE ABLE TO RECOVER IT. Would you like to save a copy of it now?"));
        if (res != QMessageBox::Yes)
            return;
    }

    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save Calibration"), "", tr("Binary (*.bin);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    file.write(reinterpret_cast<const char *>(data), static_cast<qint64>(size));
    file.close();
}

void MainWindow::displayFlashDialog()
{
    const lt::Model * model = calibration_.model();
    if (model == nullptr)
        return;
    const lt::Platform & platform = model->platform();

    FlashDialog fd;
    if (fd.exec() != QDialog::Accepted)
        return;

    lt::DataLink * link = fd.dataLink();
    if (link == nullptr)
    {
        QMessageBox::warning(this, tr("Unable to download"), tr("An interface has not been selected"));
        return;
    }

    lt::PlatformLink pl(*link, platform);
    lt::FlasherPtr flasher;
    catchWarning([&pl, &flasher]() { flasher = pl.flasher(); }, tr("Unable to flash"));

    if (!flasher)
    {
        QMessageBox::warning(this, tr("Unable to flash"),
                             tr("Unable to create a flash interface. Does the platform support reflashing and is the "
                                "proper interface selected?"));
        return;
    }

    QProgressDialog progress(tr("Flash Calibration"), tr("Cancel"), 0, 10000, this);
    progress.show();
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(false);

    flasher->setProgressCallback([&progress](float v) {
        QMetaObject::invokeMethod(&progress, "setValue", Qt::QueuedConnection,
                                  Q_ARG(int, static_cast<int>(v * 10000.f)));
    });

    connect(&progress, &QProgressDialog::canceled, [&flasher]() { flasher->cancel(); });

    calibration_.correctChecksums();

    std::atomic_bool successful(false);
    BackgroundTask<void()> bt(
        [&flasher, &successful, this]() { successful = flasher->flash(lt::FlashMap(calibration_)); });
    catchCritical(
        [&bt]() {
            bt();
            bt.future().get();
        },
        tr("Error while downloading"));

    if (!successful)
        return;

    // Save backup somewhere
    QMessageBox::information(this, tr("Reflash successful"),
                             tr("The calibration has been flashed. Please power cycle the ECU."));
}

void MainWindow::on_actionDownload_triggered() { displayDownloadDialog(); }

void MainWindow::on_actionFlash_triggered() { displayFlashDialog(); }

bool MainWindow::closeCalibration()
{
    if (!dirty_)
        return true;

    int res =
        QMessageBox::question(this, tr("Close Calibration"),
                              tr("The calibration has unsaved changes. Would you like to save it before closing?"),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (res == QMessageBox::Cancel)
        return false;
    if (res == QMessageBox::No)
        return true;

    return saveCalibration();
}

bool MainWindow::saveCalibration(bool changePath)
{
    if (!dirty_)
        return true;

    if (changePath || calibrationPath_.isEmpty())
    {
        QString path = QFileDialog::getSaveFileName(this, tr("Save Calibration"), calibrationPath_,
                                                    "Calibration (*.rom *.bin);;All (.*)");
        // If they didn't select a path, don't save.
        if (path.isEmpty())
            return false;
        QFileInfo fi(path);
        calibrationName_ = fi.baseName();
        calibrationPath_ = std::move(path);
    }

    QFile file(calibrationPath_);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, tr("Failed to save calibration"), file.errorString());
        return false;
    }

    calibration_.correctChecksums();
    file.write(reinterpret_cast<const char *>(calibration_.data()), calibration_.size());
    file.close();

    clearDirty();
    return true;
}

void MainWindow::setDirty()
{
    if (dirty_)
        return;
    dirty_ = true;
    setWindowTitle("Overboost - " + calibrationName_ + " *");
}

void MainWindow::clearDirty()
{
    dirty_ = false;
    setWindowTitle("Overboost - " + calibrationName_);
}

bool MainWindow::setCalibration(const lt::Platform * platform, const uint8_t * data, std::size_t size,
                                const QString & path)
{
    const lt::Model * model = platform->identify(data, size);
    if (model == nullptr)
        return false;

    ui->tabs->clear();

    calibration_.setModel(model);
    calibration_.setData(lt::MemoryBuffer(data, std::next(data, size)));

    if (!path.isEmpty())
    {
        QFileInfo fi(path);
        calibrationName_ = fi.baseName();
        clearDirty();
    }
    else
    {
        calibrationName_ = "Untitled Calibration";
        setDirty();
    }

    tablesModel_.setDefinition(model);
    pidsModel_.setPlatform(&model->platform());
    calibrationPath_ = path;
    return true;
}

void MainWindow::on_actionSave_triggered() { saveCalibration(); }

void MainWindow::on_actionSave_As_triggered() { saveCalibration(true); }

void MainWindow::addRecentFiles()
{
    QSettings settings;
    settings.beginGroup("QuickStart");
    QStringList history = settings.value("History").toStringList();

    // ui->actionRecent_Files->
}

void MainWindow::updateSearch()
{
    if (ui->checkRegex->isChecked())
    {
        tablesSortModel_.setFilterRegularExpression(ui->lineSearch->text());
    }
    else
    {
        tablesSortModel_.setFilterFixedString(ui->lineSearch->text());
    }
}

void MainWindow::updateRecent()
{
    QSettings settings;
    settings.beginGroup("QuickStart");
    QStringList history = settings.value("History").toStringList();
    ui->menuRecent_Files->clear();
    for (const QString & s : history)
    {
        QAction * action = ui->menuRecent_Files->addAction(s);
        connect(action, &QAction::triggered, [this, s]() { importCalibration(s); });
    }
}

void MainWindow::on_buttonModifyScale_clicked()
{
    bool ok;
    double scale = ui->lineModifier->text().toDouble(&ok);
    if (!ok)
        return;

    auto * tab = reinterpret_cast<TableView *>(ui->tabs->currentWidget());
    if (tab == nullptr)
        return;

    tab->model()->scaleAll(scale);
}

void MainWindow::on_buttonModifyAdd_clicked()
{
    bool ok;
    double amount = ui->lineModifier->text().toDouble(&ok);
    if (!ok)
        return;

    auto * tab = reinterpret_cast<TableView *>(ui->tabs->currentWidget());
    if (tab == nullptr)
        return;

    tab->model()->addAll(amount);
}
