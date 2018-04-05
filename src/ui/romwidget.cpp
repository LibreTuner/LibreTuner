#include "romwidget.h"
#include "rommanager.h"

#include <QVBoxLayout>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QStyle>
#include "createtunedialog.h"

RomWidget::RomWidget(RomPtr rom, QWidget* parent) : rom_(rom), QFrame(parent)
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    QHBoxLayout *hlayout = new QHBoxLayout();
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    
    label_ = new QLabel(QString::fromStdString(rom->name()), this);
    label_->setAlignment(Qt::AlignCenter);

    QLabel *icon = new QLabel();
    icon->setPixmap(QPixmap(":/icons/rom-file.png"));
    vlayout->addWidget(label_);
    vlayout->addLayout(hlayout);
    hlayout->addWidget(icon);
    hlayout->addLayout(buttonLayout);
    
    tuneButton_ = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Create tune");
    deleteButton_ = new QPushButton(style()->standardIcon(QStyle::SP_TrashIcon), "Delete");
    
    connect(tuneButton_, &QPushButton::clicked, this, &RomWidget::createTuneClicked);
    
    buttonLayout->addWidget(tuneButton_);
    buttonLayout->addWidget(deleteButton_);
    
    setLayout(vlayout);
    
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setLineWidth(0);
}



void RomWidget::createTuneClicked()
{
    CreateTuneDialog dlg(rom_);
    dlg.exec();
}
