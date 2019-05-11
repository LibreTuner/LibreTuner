#include "datalinkswidget.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QVBoxLayout>

#include "adddatalinkdialog.h"
#include "database/links.h"
#include "libretuner.h"

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
    linePort_ = new QLineEdit;
    comboPort_ = new QComboBox;
    lineName_->setEnabled(false);
    linePort_->setEnabled(false);
    comboPort_->setEnabled(false);

    spinBaudrate_ = new QSpinBox;
    spinBaudrate_->setVisible(false);
    spinBaudrate_->setMinimum(1);
    spinBaudrate_->setMaximum(4000000);

    linksView_ = new QTreeView;
    linksView_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksView_->setModel(&LT()->links());

    // Layouts
    auto *portLayout = new QVBoxLayout;
    portLayout->setContentsMargins(0, 0, 0, 0);
    portLayout->addWidget(comboPort_);
    portLayout->addWidget(linePort_);

    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonRemove);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), lineName_);
    formLayout->addRow(tr("Port"), portLayout);
    formLayout->addRow(tr("Baudrate"), spinBaudrate_);

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

        if (comboPort_->currentIndex() == 0) {
            link->setPort(linePort_->text().toStdString());
        } else {
            link->setPort(comboPort_->currentText().toStdString());
        }
        link->setBaudrate(spinBaudrate_->value());
        LT()->saveLinks();
        setButtonsEnabled(false);
    });

    connect(buttonReset_, &QPushButton::clicked, [this]() {
        linkChanged(currentLink());
        setButtonsEnabled(false);
    });

    connect(lineName_, &QLineEdit::textEdited,
            [this]() { setButtonsEnabled(true); });

    connect(comboPort_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index) {
                setButtonsEnabled(true);
                linePort_->setEnabled(index == 0);
            });

    connect(linePort_, &QLineEdit::textEdited,
            [this]() { setButtonsEnabled(true); });

    connect(spinBaudrate_, &QSpinBox::editingFinished,
            [this]() { setButtonsEnabled(true); });
}

void DatalinksWidget::linkChanged(lt::DataLink *link) {
    if (link == nullptr) {
        lineName_->clear();
        linePort_->clear();
        spinBaudrate_->clear();

        lineName_->setEnabled(false);
        linePort_->setEnabled(false);
        comboPort_->setEnabled(false);
        spinBaudrate_->setVisible(false);

        setButtonsEnabled(false);
        return;
    }

    lineName_->setText(QString::fromStdString(link->name()));
    linePort_->setText(QString::fromStdString(link->port()));
    spinBaudrate_->setValue(link->baudrate());
    lineName_->setEnabled(true);
    if ((link->flags() & lt::DataLinkFlags::Port) !=
    lt::DataLinkFlags::None) {
        linePort_->setEnabled(true);
        comboPort_->setEnabled(true);
        comboPort_->clear();
        std::vector<std::string> ports = link->ports();
        comboPort_->addItem("Other");

        for (const std::string &port : ports) {
            comboPort_->addItem(QString::fromStdString(port));
            if (port == link->port()) {
                comboPort_->setCurrentText(QString::fromStdString(port));
                linePort_->setEnabled(false);
                linePort_->clear();
            }
        }
    }
    spinBaudrate_->setVisible((link->flags() & lt::DataLinkFlags::Baudrate) !=
                              lt::DataLinkFlags::None);
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
