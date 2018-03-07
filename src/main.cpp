#include "zoomzoomtune.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ZoomZoomTune w;
    w.show();

    return app.exec();
}

