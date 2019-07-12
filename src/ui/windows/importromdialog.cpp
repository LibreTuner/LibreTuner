#include "importromdialog.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

#include <libretuner.h>

#include "../../database/definitions.h"

ImportRomDialog::ImportRomDialog(lt::ProjectPtr project, QWidget * parent) : QDialog(parent), project_(std::move(project))
{
    setWindowTitle(tr("LibreTuner - Download"));

    comboPlatform_ = new QComboBox;
    comboPlatform_->setModel(new PlatformsModel(&LT()->definitions(), this));

    lineName_ = new QLineEdit;

    linePath_ = new QLineEdit;
    linePath_->setClearButtonEnabled(true);
    auto * buttonBrowse = new QPushButton(style()->standardIcon(QStyle::SP_DirOpenIcon), QString());

    auto * pathLayout = new QHBoxLayout;
    pathLayout->addWidget(linePath_);
    pathLayout->addWidget(buttonBrowse);

    // Main options
    auto * form = new QFormLayout;
    form->addRow(tr("Platform"), comboPlatform_);
    form->addRow(tr("Name"), lineName_);
    form->addRow(tr("Path"), pathLayout);

    auto * groupDetails = new QGroupBox(tr("Properties"));
    groupDetails->setLayout(form);

    // Buttons
    auto * buttonImport = new QPushButton(tr("Import"));
    buttonImport->setDefault(true);
    buttonImport->setAutoDefault(true);
    auto * buttonClose = new QPushButton(tr("Close"));

    auto * buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonImport);
    buttonLayout->addWidget(buttonClose);

    // Top layout
    auto * layout = new QHBoxLayout;
    layout->addWidget(groupDetails);
    layout->addLayout(buttonLayout);

    setLayout(layout);

    connect(buttonClose, &QPushButton::clicked, this, &QWidget::hide);

    connect(comboPlatform_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportRomDialog::platformChanged);

    connect(buttonBrowse, &QPushButton::clicked, [this]()
    {
      QString dir = QFileDialog::getOpenFileName(nullptr, tr("Select raw ROM file"), linePath_->text(), tr("Binary (*.bin);;All Files (*)"));
      if (!dir.isNull())
          linePath_->setText(dir);
    });

    platformChanged(comboPlatform_->currentIndex());
}

void ImportRomDialog::platformChanged(int index)
{

}
