#ifndef CREATETUNEDIALOG_H
#define CREATETUNEDIALOG_H

#include <QDialog>

namespace Ui
{
class CreateTuneDialog;
}

/**
 * @todo write docs
 */
class CreateTuneDialog : public QDialog
{
    Q_OBJECT
public:
    CreateTuneDialog();
    
    ~CreateTuneDialog();
private:
    Ui::CreateTuneDialog* ui_;
};

#endif // CREATETUNEDIALOG_H
