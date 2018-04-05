#include "tunewidget.h"
#include "ui_tunewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "tune.h"
#include "libretuner.h"

#include <cassert>

TuneWidget::TuneWidget(TunePtr tune, QWidget *parent) : QFrame(parent), tune_(tune)
{
    assert(tune);
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    
    QLabel *title = new QLabel(QString::fromStdString(tune->name()), this);
    title->setAlignment(Qt::AlignCenter);
    vlayout->addWidget(title);
    
    QPushButton *buttonEdit = new QPushButton(style()->standardIcon(QStyle::SP_DialogOpenButton), "Edit", this);
    vlayout->addWidget(buttonEdit);
    connect(buttonEdit, &QPushButton::clicked, this, &TuneWidget::editClicked);
    QPushButton *buttonDelete = new QPushButton(style()->standardIcon(QStyle::SP_TrashIcon), "Delete", this);
    vlayout->addWidget(buttonDelete);
    
    setLayout(vlayout);
    
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setLineWidth(0);
}



void TuneWidget::editClicked()
{
    LibreTuner::get()->editTune(tune_);
}
