#include "datalinkswidget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QVBoxLayout>
#include <QCheckBox>

#include "serial/device.h"

#include "adddatalinkdialog.h"
#include "database/links.h"
#include "libretuner.h"
#include "models/serialportmodel.h"
#include "uiutil.h"
#include "widget/customcombo.h"

DatalinksWidget::DatalinksWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("LibreTuner - Datalinks"));
    resize(600, 400);

    auto *buttonAdd = new QPushButton(tr("Add"));
    auto *buttonRemove = new QPushButton(tr("Remove"));

    buttonUpdate_ = new QPushButton(tr("Save"));
    buttonUpdate_->setEnabled(false);
    buttonReset_ = new QPushButton(tr("Reset"));
    buttonReset_->setEnabled(false);

    lineName_ = new QLineEdit;
    comboPort_ = new CustomCombo;
    lineName_->setEnabled(false);
    comboPort_->setEnabled(false);

    auto *serialModel = new SerialPortModel(this);
    catchWarning(
        [serialModel]() { serialModel->setPorts(serial::enumeratePorts()); },
        tr("Error enumerating serial ports"));
    comboPort_->setModel(serialModel);

    spinBaudrate_ = new QSpinBox;
    spinBaudrate_->setVisible(false);
    spinBaudrate_->setMinimum(1);
    spinBaudrate_->setMaximum(4000000);

    checkBaudrate_ = new QCheckBox(tr("Change baudrate"));
    checkBaudrate_->setChecked(true);

    linksView_ = new QTreeView;
    linksView_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksView_->setModel(&LT()->links());

    // Layouts
    auto *portLayout = new QVBoxLayout;
    portLayout->setContentsMargins(0, 0, 0, 0);
    portLayout->addWidget(comboPort_);

    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRemove);

    auto *baudrateLayout = new QHBoxLayout;
    baudrateLayout->setContentsMargins(0, 0, 0, 0);
    baudrateLayout->addWidget(spinBaudrate_);
    baudrateLayout->addWidget(checkBaudrate_);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), lineName_);
    formLayout->addRow(tr("Port"), portLayout);
    formLayout->addRow(tr("Baudrate"), baudrateLayout);

    auto *layoutOptButtons = new QVBoxLayout;
    layoutOptButtons->setAlignment(Qt::AlignTop);
    layoutOptButtons->addWidget(buttonUpdate_);
    layoutOptButtons->addWidget(buttonReset_);

    auto *layoutOpt = new QHBoxLayout;
    layoutOpt->addLayout(formLayout);
    layoutOpt->addLayout(layoutOptButtons);

    auto *linksLayout = new QVBoxLayout;
    linksLayout->addWidget(linksView_);
    linksLayout->addLayout(layoutOpt);

    auto *layout = new QHBoxLayout;
    layout->addLayout(linksLayout);
    layout->addLayout(buttonLayout);
    setLayout(layout);

    connect(buttonAdd, &QPushButton::clicked, []() {
        AddDatalinkDialog dlg;
        dlg.exec();
    });

    connect(buttonRemove, &QPushButton::clicked, [this]() {
        auto *link = currentLink();
        if (link == nullptr) {
            return;
        }
        LT()->links().remove(link);
        LT()->saveLinks();
    });

    connect(
        linksView_, &QTreeView::activated,
        [this](const QModelIndex & /*index*/) { linkChanged(currentLink()); });

    connect(buttonUpdate_, &QPushButton::clicked, [this]() {
        lt::DataLink *link = currentLink();
        if (link == nullptr) {
            return;
        }

        link->setName(lineName_->text().toStdString());
        link->setPort(comboPort_->value().toStdString());
        link->setBaudrate(checkBaudrate_->isChecked() ? spinBaudrate_->value() : 0);
        LT()->saveLinks();
        setButtonsEnabled(false);
    });

    connect(buttonReset_, &QPushButton::clicked, [this]() {
        linkChanged(currentLink());
        setButtonsEnabled(false);
    });

    connect(checkBaudrate_, &QCheckBox::stateChanged, [this](int state) {
      spinBaudrate_->setEnabled(state == Qt::Checked);
    });

    connect(lineName_, &QLineEdit::textEdited,
            [this]() { setButtonsEnabled(true); });

    connect(comboPort_, &CustomCombo::valueChanged,
            [this]() { setButtonsEnabled(true); });

    connect(spinBaudrate_, &QSpinBox::editingFinished,
            [this]() { setButtonsEnabled(true); });
}

void DatalinksWidget::linkChanged(lt::DataLink *link) {
    if (link == nullptr) {
        lineName_->clear();
        spinBaudrate_->clear();

        lineName_->setEnabled(false);
        comboPort_->setEnabled(false);
        spinBaudrate_->setVisible(false);

        setButtonsEnabled(false);
        return;
    }

    lineName_->setText(QString::fromStdString(link->name()));
    spinBaudrate_->setValue(link->baudrate());
    lineName_->setEnabled(true);
    if ((link->flags() & lt::DataLinkFlags::Port) != lt::DataLinkFlags::None) {
        comboPort_->setEnabled(true);
        comboPort_->setValue(QString::fromStdString(link->port()));
    }

    bool baudrateSupported = (link->flags() & lt::DataLinkFlags::Baudrate) !=
                             lt::DataLinkFlags::None;
    spinBaudrate_->setVisible(baudrateSupported);
    checkBaudrate_->setVisible(baudrateSupported);
    checkBaudrate_->setChecked(link->baudrate() != 0);
    if (link->baudrate() == 0) {
        spinBaudrate_->setEnabled(false);
    }

    setButtonsEnabled(false);
}

lt::DataLink *DatalinksWidget::currentLink() const {
    QVariant data =
        LT()->links().data(linksView_->currentIndex(), Qt::UserRole);
    if (!data.canConvert<lt::DataLink *>()) {
        return nullptr;
    }

    return data.value<lt::DataLink *>();
}
void DatalinksWidget::setButtonsEnabled(bool enabled) {
    buttonUpdate_->setEnabled(enabled);
    buttonReset_->setEnabled(enabled);
}
