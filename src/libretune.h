#ifndef LIBRETUNE_H
#define LIBRETUNE_H

#include <QApplication>
#include "ui/mainwindow.h"

#include <memory>

class CanHandler;

class LibreTune : public QApplication
{
    Q_OBJECT
public:
    LibreTune(int &argc, char *argv[]);
    
    /* Returns the global LibreTune object */
    static LibreTune *get();
    
    /* Returns the CAN handler. Returns nullptr if a handler
     * does not exist */
    CanHandler *canHandler()
    {
        return canHandler_.get();
    }

    ~LibreTune();
    

private:
    std::unique_ptr<MainWindow> mainWindow_;
    std::unique_ptr<CanHandler> canHandler_;
    
    
signals:
    void canHandlerChanged(CanHandler *handler);
};

#endif // LIBRETUNE_H
