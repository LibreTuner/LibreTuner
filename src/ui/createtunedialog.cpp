#include "createtunedialog.h"
#include "ui_createtunedialog.h"

#include "rommanager.h"
#include <tunemanager.h>
#include <QMessageBox>

Q_DECLARE_METATYPE(RomPtr)

CreateTuneDialog::CreateTuneDialog(RomPtr base) : ui_(new Ui::CreateTuneDialog)
{
    ui_->setupUi(this);
    
    for (RomPtr &rom : RomManager::get()->roms())
    {
        ui_->comboBase->addItem(QString::fromStdString(rom->name()), QVariant::fromValue(rom));
        if (base != nullptr && rom == base)
        {
            ui_->comboBase->setCurrentIndex(ui_->comboBase->count() - 1);
        }
    }
}



CreateTuneDialog::~CreateTuneDialog()
{
    delete ui_;
}



void CreateTuneDialog::on_buttonCreate_clicked()
{
    if (!TuneManager::get()->createTune(ui_->comboBase->currentData().value<RomPtr>(), ui_->lineName->text().toStdString()))
    {
        QMessageBox msgBox;
        msgBox.setText("Error while creating tune: " + TuneManager::get()->lastError());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Tune creation error");
        msgBox.exec();
    }
    
    close();
}
