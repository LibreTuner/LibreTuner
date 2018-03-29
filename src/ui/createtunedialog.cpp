#include "createtunedialog.h"
#include "ui_createtunedialog.h"

#include "rommanager.h"

Q_DECLARE_METATYPE(RomDataPtr)

CreateTuneDialog::CreateTuneDialog() : ui_(new Ui::CreateTuneDialog)
{
    ui_->setupUi(this);
    
    for (RomDataPtr &rom : RomManager::get()->roms())
    {
        ui_->comboBase->addItem(QString::fromStdString(rom->name()), QVariant::fromValue(rom));
    }
}



CreateTuneDialog::~CreateTuneDialog()
{
    delete ui_;
}
