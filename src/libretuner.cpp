#include "libretune.h"
#include "os/sockethandler.h"
#include "protocols/socketcaninterface.h"
#include "protocols/isotpinterface.h"


static LibreTune *_global;


LibreTune::LibreTune(int& argc, char *argv[]) : QApplication(argc, argv)
{
    _global = this;
    
    mainWindow_ = std::unique_ptr<MainWindow>(new MainWindow);
    mainWindow_->show();

    
    SocketHandler::get()->initialize();
}



LibreTune * LibreTune::get()
{
    return _global;
}



LibreTune::~LibreTune()
{
}
