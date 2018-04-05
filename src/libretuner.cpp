#include "libretuner.h"
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#include "protocols/isotpinterface.h"
#include "rommanager.h"
#include "ui/tuneeditor.h"
#include "tune.h"

#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include "tunemanager.h"


static LibreTuner *_global;


LibreTuner::LibreTuner(int& argc, char *argv[]) : QApplication(argc, argv)
{
    _global = this;
    home_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    SocketHandler::get()->initialize();
    if (!RomManager::get()->load())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load ROM metadata from roms.xml: " + RomManager::get()->lastError());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("RomManager error");
        msgBox.exec();
    }
    
    if (!TuneManager::get()->load())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load tune metadata from tunes.xml: " + RomManager::get()->lastError());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("TuneManager error");
        msgBox.exec();
    }
    
    mainWindow_ = std::unique_ptr<MainWindow>(new MainWindow);
    mainWindow_->show();
    
    checkHome();
}



void LibreTuner::editTune(TunePtr tune)
{
    TuneDataPtr data = std::make_shared<TuneData>(tune);
    if (!data->valid())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Tune data error");
        msgBox.setText(QStringLiteral("Error opening tune: ") + QString::fromStdString(data->lastError()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    tuneEditor_.reset(new TuneEditor(data));
    tuneEditor_->show();
}



LibreTuner * LibreTuner::get()
{
    return _global;
}



LibreTuner::~LibreTuner()
{
}



void LibreTuner::checkHome()
{
    QDir home(home_);
    home.mkpath(".");
    home.mkdir("roms");
    home.mkdir("tunes");
}
