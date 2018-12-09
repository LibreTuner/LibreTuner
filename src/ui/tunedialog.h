#ifndef TUNEDIALOG_H
#define TUNEDIALOG_H

#include <QDialog>


class RomsView;
class Tune;

class TuneDialog : public QDialog
{
public:
    explicit TuneDialog(QWidget *parent = nullptr);
    
    Tune *selectedTune();
    
private:
    RomsView *roms_;
};

#endif // TUNEDIALOG_H
