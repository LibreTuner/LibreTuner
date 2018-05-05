#ifndef TUNEWIDGET_H
#define TUNEWIDGET_H

#include <QWidget>
#include <QFrame>

#include <memory>

class Tune;
typedef std::shared_ptr<Tune> TunePtr;

/**
 * @todo write docs
 */
class TuneWidget : public QFrame
{
    Q_OBJECT
public:
    TuneWidget(TunePtr tune, QWidget *parent = 0);

private:
    TunePtr tune_;
    
private slots:
    void editClicked();
    void flashClicked();
};

#endif // TUNEWIDGET_H
