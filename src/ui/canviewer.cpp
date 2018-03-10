#include "canviewer.h"
#include "ui_canviewer.h"
#include "../protocols/canlog.h"
#include "../libretune.h"
#include "../protocols/canhandler.h"
#include "canlogview.h"

#include <QAbstractItemModel>

CanViewer::CanViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CanViewer)
{
    ui->setupUi(this);
    
    connect(LibreTune::get(), &LibreTune::canHandlerChanged, this, &CanViewer::canHandlerChanged);
    
    CanHandler *handler = LibreTune::get()->canHandler();
    if (handler)
    {
        logModel_ = handler->log();
        ui->logView->setModel(logModel_);
        connect(logModel_, &QAbstractItemModel::rowsInserted, this, &CanViewer::rowsInserted);
    }
}



CanViewer::~CanViewer()
{
    delete ui;
}



void CanViewer::rowsInserted(const QModelIndex& parent, int first, int last)
{
    if (ui->autoScroll->isChecked())
    {
        ui->logView->scrollToBottom();
    }
}



void CanViewer::canHandlerChanged(CanHandler* handler)
{
    if (logModel_)
    {
        disconnect(logModel_, &QAbstractItemModel::rowsInserted, this, &CanViewer::rowsInserted);
    }
    if (handler)
    {
        logModel_ = handler->log();
        connect(logModel_, &QAbstractItemModel::rowsInserted, this, &CanViewer::rowsInserted);
    }
    else
    {
        logModel_ = nullptr;
    }
    ui->logView->setModel(logModel_);
}
