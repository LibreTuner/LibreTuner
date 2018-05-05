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
    
#ifdef WITH_SOCKETCAN
    SocketHandler::get()->initialize();
#endif
    
    if (!DefinitionManager::get()->load())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load definitions: " + QString::fromStdString(DefinitionManager::get()->lastError()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("DefinitionManager error");
        msgBox.exec();
    }
    
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



void LibreTuner::flashTune(TunePtr tune)
{
    
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
    
    if (!home.exists("definitions")) 
    {
        home.mkdir("definitions");
        // Copy definitions
        QDir dDir(":/definitions");
        
        for (QFileInfo &info : dDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort))
        {
            QDir realDefDir(home.path() + "/definitions/" + info.fileName() + "/");
            realDefDir.mkpath(".");
            QDir subDir(info.filePath());
            for (QFileInfo &i : subDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::NoSort))
            {
                QFile::copy(i.filePath(), realDefDir.path() + "/" + i.fileName());
            }
        }
    }
}
