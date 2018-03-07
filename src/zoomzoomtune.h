#ifndef ZOOMZOOMTUNE_H
#define ZOOMZOOMTUNE_H

#include <QMainWindow>

namespace Ui {
class ZoomZoomTune;
}

class ZoomZoomTune : public QMainWindow
{
    Q_OBJECT

public:
    explicit ZoomZoomTune(QWidget *parent = 0);
    ~ZoomZoomTune();

private:
    Ui::ZoomZoomTune *ui;
};

#endif // ZOOMZOOMTUNE_H
