#include "importromdialog.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <lt/project/project.h>

#include <fstream>

#include "database/definitions.h"
#include "libretuner.h"
#include "uiutil.h"

ImportRomDialog::ImportRomDialog(lt::ProjectPtr project, QWidget * parent)
    : QDialog(parent), project_(std::move(project))
{
    setWindowTitle(tr("LibreTuner - Import ROM"));

    comboPlatform_ = new QComboBox;
    comboPlatform_->setModel(new PlatformsModel(&LT()->definitions(), this));

    lineName_ = new QLineEdit;

    linePath_ = new QLineEdit;
    linePath_->setClearButtonEnabled(true);
    auto * buttonBrowse = new QPushButton(
        style()->standardIcon(QStyle::SP_DirOpenIcon), QString());

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

    connect(buttonClose, &QPushButton::clicked, this, &QDialog::reject);

    connect(comboPlatform_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportRomDialog::platformChanged);

    connect(buttonBrowse, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(
            nullptr, tr("Select raw ROM file"), linePath_->text(),
            tr("Binary (*.bin);;All Files (*)"));
        if (!path.isNull())
        {
            linePath_->setText(path);

            if (lineName_->text().isEmpty())
                lineName_->setText(QString::fromStdString(
                    std::filesystem::path(path.toStdString())
                        .filename()
                        .string()));
        }
    });

    platformChanged(comboPlatform_->currentIndex());

    connect(buttonImport, &QPushButton::clicked, [this]() {
        QVariant var = comboPlatform_->currentData(Qt::UserRole);
        if (!var.canConvert<lt::PlatformPtr>())
            return;

        auto platform = var.value<lt::PlatformPtr>();

        catchWarning(
            [&]() {
                // Import and save ROM
                project_
                    ->importRom(lineName_->text().toStdString(),
                                linePath_->text().toStdString(), platform)
                    ->save();
                accept();
            },
            tr("Error importing ROM"));
    });
}

void ImportRomDialog::platformChanged(int index) {}
