#include "tunedialog.h"

#include "romsview.h"

#include <QPushButton>
#include <QVBoxLayout>

TuneDialog::TuneDialog(QWidget * parent) : QDialog(parent)
{
    setWindowTitle(tr("Select Tune"));
    resize(600, 400);

    roms_ = new RomsView;
    // roms_->setModel(new RomsModel(this));

    QPushButton * select = new QPushButton(tr("Select"));
    select->setEnabled(false);
    select->setDefault(true);
    select->setAutoDefault(true);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(roms_);
    layout->addWidget(select);

    setLayout(layout);

    // Signals

    connect(roms_, &RomsView::activated, [this](const QModelIndex & index) {
        if (roms_->selectedTune() != nullptr)
        {
            close();
        }
    });

    connect(roms_, &RomsView::tuneChanged, [this, select]() {
        select->setEnabled(roms_->selectedTune() != nullptr);
    });

    connect(select, &QPushButton::clicked, this, &QWidget::close);
}

Tune * TuneDialog::selectedTune() { return roms_->selectedTune(); }
