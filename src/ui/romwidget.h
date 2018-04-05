#ifndef ROMWIDGET_H
#define ROMWIDGET_H

#include <memory>

#include <QFrame>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>

class QPushButton;

class Rom;
typedef std::shared_ptr<Rom> RomPtr;

/**
 * Widget for the ROM list
 */
class RomWidget : public QFrame
{
    Q_OBJECT
public:
    explicit RomWidget(RomPtr rom, QWidget *parent = 0);

private:
    QLabel *label_;
    QPushButton *deleteButton_;
    QPushButton *tuneButton_;
    
    RomPtr rom_;
    
public slots:
    void createTuneClicked();
};

#endif // ROMWIDGET_H
