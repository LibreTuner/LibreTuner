#ifndef LIBRETUNE_H
#define LIBRETUNE_H

#include <QApplication>
#include "ui/mainwindow.h"
#include "protocols/canlog.h"

#include <memory>

class IsoTpInterface;
class IsoTpTest;

class LibreTuner : public QApplication
{
    Q_OBJECT
public:
    LibreTuner(int &argc, char *argv[]);
    
    /* Returns the global LibreTune object */
    static LibreTuner *get();
    
    /* Returns the CAN log. */
    CanLog *canLog()
    {
        return &canLog_;
    }

    ~LibreTuner();
    
    /* Checks if the home directory exists and if it does not,
     * creates it. */
    void checkHome();
    
    QString home() {
        return home_;
    }

private:
    std::unique_ptr<MainWindow> mainWindow_;
    CanLog canLog_;
    
    /* Location of home directory. */
    QString home_;
};

#endif // LIBRETUNE_H
