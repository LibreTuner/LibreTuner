#include "tunedialog.h"

#include "rom.h"
#include "romsview.h"

#include <QVBoxLayout>
#include <QPushButton>


TuneDialog::TuneDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Select Tune"));
    QVBoxLayout *layout = new QVBoxLayout;
    
    roms_ = new RomsView;
    roms_->setModel(new RomsModel(this));
    connect(roms_, &RomsView::activated, [this](const QModelIndex &index) {
        if (roms_->selectedTune() != nullptr) {
            close();
        }
    });
    
    QPushButton *select = new QPushButton(tr("Select"));
    select->setEnabled(false);
    
    connect(roms_, &RomsView::tuneChanged, [this, select]() {
        select->setEnabled(roms_->selectedTune() != nullptr);
    });
    
    layout->addWidget(roms_);
    layout->addWidget(select);
    
    setLayout(layout);
}



Tune *TuneDialog::selectedTune()
{
    return roms_->selectedTune();
}
