#include "libretuner.h"
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#include "protocols/isotpinterface.h"
#include "rommanager.h"

#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>


static LibreTuner *_global;


LibreTuner::LibreTuner(int& argc, char *argv[]) : QApplication(argc, argv)
{
    _global = this;
    home_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    SocketHandler::get()->initialize();
    if (!RomManager::get()->load())
    {
        QMessageBox msgBox;
        msgBox.setText("Could not load ROMs: " + RomManager::get()->lastError());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("RomManager error");
        msgBox.exec();
    }
    
    mainWindow_ = std::unique_ptr<MainWindow>(new MainWindow);
    mainWindow_->show();
    
    checkHome();
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
}
