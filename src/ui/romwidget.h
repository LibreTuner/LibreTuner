#ifndef ROMWIDGET_H
#define ROMWIDGET_H

#include <memory>

#include <QFrame>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>

class QPushButton;

class RomData;
typedef std::shared_ptr<RomData> RomDataPtr;

/**
 * Widget for the ROM list
 */
class RomWidget : public QFrame
{
    Q_OBJECT
public:
    explicit RomWidget(RomDataPtr rom, QWidget *parent = 0);

private:
    QLabel *label_;
    QPushButton *deleteButton_;
    QPushButton *tuneButton_;
    
    RomDataPtr rom_;
    
public slots:
    void createTuneClicked();
};

#endif // ROMWIDGET_H
