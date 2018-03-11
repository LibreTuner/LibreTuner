#ifndef ROMWIDGET_H
#define ROMWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>

/**
 * Widget for the ROM list
 */
class RomWidget : public QWidget
{
public:
    explicit RomWidget(QWidget *parent = 0);
    
private:
    QLabel *label_;
    QGraphicsView *image_;
    QGraphicsScene scene_;
};

#endif // ROMWIDGET_H
