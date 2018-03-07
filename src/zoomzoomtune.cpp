#include "zoomzoomtune.h"
#include "ui_zoomzoomtune.h"

ZoomZoomTune::ZoomZoomTune(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ZoomZoomTune)
{
    ui->setupUi(this);
}

ZoomZoomTune::~ZoomZoomTune()
{
    delete ui;
}
