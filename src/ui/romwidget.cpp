#include "romwidget.h"
#include <QVBoxLayout>
#include <QGraphicsPixmapItem>

RomWidget::RomWidget(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    label_ = new QLabel("N/A", this);
    label_->setAlignment(Qt::AlignCenter);
    
    scene_.addItem(new QGraphicsPixmapItem(QPixmap(":/icons/rom-file.png")));
    
    image_ = new QGraphicsView(&scene_, this);
    layout->addWidget(image_);
    layout->addWidget(label_);
    
    setLayout(layout);
}


