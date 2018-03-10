#ifndef LIBRETUNE_H
#define LIBRETUNE_H

#include <QApplication>
#include "ui/mainwindow.h"
#include "protocols/canlog.h"

#include <memory>

class IsoTpInterface;
class IsoTpTest;

class LibreTune : public QApplication
{
    Q_OBJECT
public:
    LibreTune(int &argc, char *argv[]);
    
    /* Returns the global LibreTune object */
    static LibreTune *get();
    
    /* Returns the CAN log. */
    CanLog *canLog()
    {
        return &canLog_;
    }

    ~LibreTune();
    

private:
    std::unique_ptr<MainWindow> mainWindow_;
    CanLog canLog_;
};

#endif // LIBRETUNE_H
