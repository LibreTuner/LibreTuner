#include "libretune.h"
#include "protocols/canhandler.h"
#include "os/sockethandler.h"


static LibreTune *_global;


LibreTune::LibreTune(int& argc, char *argv[]) : QApplication(argc, argv)
{
    _global = this;
    
    canHandler_ = std::unique_ptr<CanHandler>(new CanHandler);
    emit canHandlerChanged(canHandler_.get());
    
    mainWindow_ = std::unique_ptr<MainWindow>(new MainWindow);
    mainWindow_->show();
    
    
    canHandler()->startSocketCan("vcan0");
    
    SocketHandler::get()->initialize();
}



LibreTune * LibreTune::get()
{
    return _global;
}



LibreTune::~LibreTune()
{

}
