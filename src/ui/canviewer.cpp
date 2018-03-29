#include "canviewer.h"
#include "ui_canviewer.h"
#include "protocols/canlog.h"
#include "libretuner.h"
#include "canlogview.h"

#include <QAbstractItemModel>

CanViewer::CanViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CanViewer)
{
    ui->setupUi(this);

    logModel_ = LibreTuner::get()->canLog();
    ui->logView->setModel(logModel_);
    connect(logModel_, &QAbstractItemModel::rowsInserted, this, &CanViewer::rowsInserted);
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
