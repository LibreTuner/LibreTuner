#include "adddatalinkdialog.h"

#include "libretuner.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "../models/serialportmodel.h"
#include "uiutil.h"
#include "widget/customcombo.h"
#include "widget/datalinksettings.h"

#ifdef WITH_SOCKETCAN
#include "lt/link/socketcan.h"
#endif
#include "lt/link/elm.h"

AddDatalinkDialog::AddDatalinkDialog(QWidget * parent) : QDialog(parent)
{
    setWindowTitle(tr("LibreTuner - Add Datalink"));

    // Type combo
    comboType_ = new QComboBox;
    comboType_->addItem("SocketCAN");
    comboType_->addItem("ELM327/ST");

    // Settings
    settings_ = new DataLinkSettings;

    // Buttons
    auto * buttonAdd = new QPushButton(tr("Add"));
    auto * buttonCancel = new QPushButton(tr("Cancel"));

    // Form layout
    auto * form = new QVBoxLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->addWidget(comboType_);
    form->addWidget(settings_);

    // Button layout
    auto * buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonCancel);
    buttonLayout->setAlignment(Qt::AlignTop);

    // Main layout
    auto * layout = new QHBoxLayout;
    layout->addLayout(form);
    layout->addLayout(buttonLayout);

    setLayout(layout);

    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::close);
    connect(buttonAdd, &QPushButton::clicked, this,
            &AddDatalinkDialog::addClicked);

    connect(comboType_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddDatalinkDialog::typeIndexChanged);
    typeIndexChanged(0);
}

void AddDatalinkDialog::addClicked()
{
    std::string name = settings_->name().trimmed().toStdString();
    if (name.empty())
    {
        QMessageBox::warning(this, tr("Invalid name"),
                             tr("Datalink name must not be empty"));
        return;
    }

    switch (comboType_->currentIndex())
    {
    case 0:
        // SocketCAN
#ifdef WITH_SOCKETCAN
        LT()->links().add(std::make_unique<lt::SocketCanLink>(
            name, settings_->port().toStdString()));
        LT()->saveLinks();
        close();
#endif
        break;
    case 1:
        // ELM327
        LT()->links().add(std::make_unique<lt::ElmDataLink>(
            name, settings_->port().toStdString(), settings_->baudrate()));
        LT()->saveLinks();
        close();
        break;
    default:
        QMessageBox::warning(
            this, tr("Unsupported type"),
            tr("The selected datalink type is unsupported on this platform"));
        break;
    }
}

void AddDatalinkDialog::typeIndexChanged(int index)
{
    switch (index)
    {
    case 0: // SocketCAN
        settings_->setFlags(lt::DataLinkFlags::Port |
                            lt::DataLinkFlags::Baudrate);
        settings_->setPortType(lt::DataLinkPortType::NetworkCan);
        break;
    case 1: // ELM
        settings_->setFlags(lt::DataLinkFlags::Port |
                            lt::DataLinkFlags::Baudrate);
        settings_->setPortType(lt::DataLinkPortType::Serial);
        break;
    default:
        settings_->setFlags(lt::DataLinkFlags::None);
    }
    settings_->setPort("");
}
