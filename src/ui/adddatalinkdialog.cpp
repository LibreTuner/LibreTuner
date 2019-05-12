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

#ifdef WITH_SOCKETCAN
#include "lt/link/socketcan.h"
#endif
#include "lt/link/elm.h"

AddDatalinkDialog::AddDatalinkDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("LibreTuner - Add Datalink"));

    // Form
    comboType_ = new QComboBox;
    comboType_->addItem("SocketCAN");
    comboType_->addItem("ELM327/ST");

    lineName_ = new QLineEdit;
    linePort_ = new QLineEdit;
    spinBaudrate_ = new QSpinBox;
    spinBaudrate_->setRange(0, 4000000);
    spinBaudrate_->setValue(115200);
    spinBaudrate_->setEnabled(false);

    // Buttons
    auto *buttonAdd = new QPushButton(tr("Add"));
    auto *buttonCancel = new QPushButton(tr("Cancel"));

    // Form layout
    auto *form = new QFormLayout;
    form->addRow(tr("Type"), comboType_);
    form->addRow(tr("Name"), lineName_);
    form->addRow(tr("Port"), linePort_);
    form->addRow(tr("Baudrate"), spinBaudrate_);

    // Button layout
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(buttonAdd);
    buttonLayout->addWidget(buttonCancel);
    buttonLayout->setAlignment(Qt::AlignTop);

    // Main layout
    auto *layout = new QHBoxLayout;
    layout->addLayout(form);
    layout->addLayout(buttonLayout);

    setLayout(layout);

    connect(buttonCancel, &QPushButton::clicked, this, &QDialog::close);
    connect(buttonAdd, &QPushButton::clicked, this,
            &AddDatalinkDialog::addClicked);

    connect(comboType_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                spinBaudrate_->setEnabled(index == 1); // Only enable for ELM327
            });
}

void AddDatalinkDialog::addClicked() {
    QString name = lineName_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid name"),
                             tr("Datalink name must not be empty"));
        return;
    }

    std::string name_s = name.toStdString();
    std::string port = linePort_->text().trimmed().toStdString();

    switch (comboType_->currentIndex()) {
    case 0:
        // SocketCAN
#ifdef WITH_SOCKETCAN
        LT()->links().add(std::make_unique<lt::SocketCanLink>(name_s, port));
        LT()->saveLinks();
        close();
#endif
        break;
    case 1:
        // ELM327
        LT()->links().add(std::make_unique<lt::ElmDataLink>(name_s, port, spinBaudrate_->value()));
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
