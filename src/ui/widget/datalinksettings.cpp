#include "datalinksettings.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include <lt/os/netif_enum.h>
#include <serial/device.h>

#include "models/serialportmodel.h"
#include "ui/widget/customcombo.h"
#include "uiutil.h"

#ifdef __linux__
#include <sys/socket.h>
#endif

DataLinkSettings::DataLinkSettings(lt::DataLinkFlags flags, QWidget *parent)
    : QWidget(parent) {
    // Name
    lineName_ = new QLineEdit;

    // Port
    comboPort_ = new CustomCombo;
    portModel_ = new SerialPortModel(this);
    comboPort_->setModel(portModel_);
    labelPort_ = new QLabel(tr("Port"));

    // Baudrate
    spinBaudrate_ = new QSpinBox;
    spinBaudrate_->setVisible(false);
    spinBaudrate_->setRange(0, 4000000);
    spinBaudrate_->setValue(115200);
    spinBaudrate_->setSpecialValueText(tr("Unchanged"));
    spinBaudrate_->setEnabled(false);

    checkBaudrate_ = new QCheckBox(tr("Use system baudrate"));
    checkBaudrate_->setChecked(true);

    labelBaudrate_ = new QLabel(tr("Baudrate"));

    baudrateLayout_ = new QHBoxLayout;
    baudrateLayout_->setContentsMargins(0, 0, 0, 0);
    baudrateLayout_->addWidget(spinBaudrate_);
    baudrateLayout_->addWidget(checkBaudrate_);

    // Form
    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);

    form->addRow(tr("Name"), lineName_);
    form->addRow(labelPort_, comboPort_);
    form->addRow(labelBaudrate_, baudrateLayout_);

    setFlags(flags);

    setLayout(form);

    connect(checkBaudrate_, &QCheckBox::stateChanged, [this](int state) {
        spinBaudrate_->setEnabled(state == Qt::Unchecked);
    });

    connect(lineName_, &QLineEdit::textEdited,
            [this]() { emit settingChanged(); });

    connect(comboPort_, &CustomCombo::valueChanged,
            [this]() { emit settingChanged(); });

    connect(spinBaudrate_, &QSpinBox::editingFinished,
            [this]() { emit settingChanged(); });

    connect(checkBaudrate_, &QCheckBox::stateChanged,
            [this](int /*state*/) { emit settingChanged(); });
}

void DataLinkSettings::setFlags(lt::DataLinkFlags flags) {
    bool baudrate =
        (flags & lt::DataLinkFlags::Baudrate) != lt::DataLinkFlags::None;
    bool port = (flags & lt::DataLinkFlags::Port) != lt::DataLinkFlags::None;

    labelPort_->setVisible(port);
    comboPort_->setVisible(port);
    labelBaudrate_->setVisible(baudrate);
    spinBaudrate_->setVisible(baudrate);
    checkBaudrate_->setVisible(baudrate);
}

void DataLinkSettings::apply(lt::DataLink &link) {
    link.setName(lineName_->text().toStdString());
    link.setPort(comboPort_->value().toStdString());
    link.setBaudrate(checkBaudrate_->isChecked() ? 0 : spinBaudrate_->value());
}

void DataLinkSettings::fill(lt::DataLink *link) {
    if (link == nullptr) {
        reset();
        return;
    }

    if ((link->flags() & lt::DataLinkFlags::Port) != lt::DataLinkFlags::None) {
        setPortType(link->portType());
    } else {
        portModel_->setPorts(std::vector<std::string>());
    }

    lineName_->setText(QString::fromStdString(link->name()));
    comboPort_->setValue(QString::fromStdString(link->port()));
    spinBaudrate_->setValue(link->baudrate());
    checkBaudrate_->setChecked(link->baudrate() == 0);
}

void DataLinkSettings::reset() {
    lineName_->clear();
    comboPort_->setValue("");
    spinBaudrate_->setValue(0);
    checkBaudrate_->setChecked(true);
}

QString DataLinkSettings::name() const { return lineName_->text(); }

QString DataLinkSettings::port() const { return comboPort_->value(); }

int DataLinkSettings::baudrate() const {
    return (checkBaudrate_->isChecked() ? 0 : spinBaudrate_->value());
}

void DataLinkSettings::setPortType(lt::DataLinkPortType type) {
    std::vector<std::string> ports;
    switch (type) {
    case lt::DataLinkPortType::Serial:
        ports = serial::enumeratePorts();
        break;
    case lt::DataLinkPortType::NetworkCan:
#ifdef __linux__
        ports = lt::enumerateNetworkInterfaces(/*AF_CAN*/);
#endif
        break;
    }

    catchWarning(
        [this, ports{std::move(ports)}]() mutable {
            portModel_->setPorts(std::move(ports));
        },
        tr("Error enumerating ports"));
}

void DataLinkSettings::setPort(const QString &port) {
    comboPort_->setValue(port);
}
