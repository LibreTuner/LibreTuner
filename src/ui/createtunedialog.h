#ifndef CREATETUNEDIALOG_H
#define CREATETUNEDIALOG_H

#include <QDialog>

#include <memory>

namespace Ui
{
class CreateTuneDialog;
}

class Rom;
typedef std::shared_ptr<Rom> RomPtr;

/**
 * @todo write docs
 */
class CreateTuneDialog : public QDialog
{
    Q_OBJECT
public:
    CreateTuneDialog(RomPtr base = nullptr);
    
    ~CreateTuneDialog();
private:
    Ui::CreateTuneDialog* ui_;
    
private slots:
    void on_buttonCreate_clicked();
};

#endif // CREATETUNEDIALOG_H
