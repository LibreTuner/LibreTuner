#include "DataLoggerWidget.h"
#include "ui_DataLoggerWidget.h"

DataLoggerWidget::DataLoggerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataLoggerWidget)
{
    ui->setupUi(this);
}

DataLoggerWidget::~DataLoggerWidget()
{
    delete ui;
}
