#include "interfaceswindow.h"
#include "ui_interfaceswindow.h"

#include "addinterfacedialog.h"
#include "settingswidget.h"

#include <QMessageBox>

Q_DECLARE_METATYPE(InterfaceSettingsPtr);

InterfacesWindow::InterfacesWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::InterfacesWindow) {
  ui->setupUi(this);
  setWindowFlags(Qt::Window);
  conn_ = InterfaceManager::get().connect(std::bind(
      &InterfacesWindow::interfacesChanged, this, std::placeholders::_1));
  interfacesChanged(InterfaceManager::get().settings());
}

InterfacesWindow::~InterfacesWindow() { delete ui; }

void InterfacesWindow::on_buttonAdd_clicked() {
  AddInterfaceDialog dlg;
  dlg.exec();
}

void InterfacesWindow::on_buttonRemove_clicked() {
  QListWidgetItem *item = ui->listInterfaces->currentItem();
  if (item == nullptr) {
    return;
  }

  auto interface = item->data(Qt::UserRole).value<InterfaceSettingsPtr>();

  QMessageBox msg;
  msg.setText("Are you sure you want to remove the interface '" +
              QString::fromStdString(interface->name()) + "'?");
  msg.setWindowTitle("Remove Interface");
  msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
  msg.setIcon(QMessageBox::Question);
  msg.setDefaultButton(QMessageBox::Cancel);
  if (msg.exec() == QMessageBox::Yes) {
    InterfaceManager::get().remove(interface);
  }
}

void InterfacesWindow::interfacesChanged(
    gsl::span<const InterfaceSettingsPtr> interfaces) {
  ui->listInterfaces->clear();

  for (const auto &iface : interfaces) {
    QListWidgetItem *item =
        new QListWidgetItem(QString::fromStdString(iface->name()));
    item->setData(Qt::UserRole,
                  QVariant::fromValue<InterfaceSettingsPtr>(iface));
    ui->listInterfaces->addItem(item);
  }
}

void InterfacesWindow::on_listInterfaces_currentItemChanged(
    QListWidgetItem *current, QListWidgetItem *previous) {
  if (current == nullptr) {
    replaceSettings(nullptr);
    return;
  }
  InterfaceSettingsPtr iface =
      current->data(Qt::UserRole).value<InterfaceSettingsPtr>();
  if (!iface) {
    replaceSettings(nullptr);
    return;
  }
  replaceSettings(SettingsWidget::create(iface));
}

void InterfacesWindow::replaceSettings(std::unique_ptr<SettingsWidget> widget) {
  if (settings_)
    ui->settingsLayout->removeWidget(settings_.get());
  settings_ = std::move(widget);
  if (settings_) {
    ui->settingsLayout->addWidget(settings_.get());
    ui->buttonApply->setEnabled(true);
  } else {
    ui->buttonApply->setEnabled(false);
  }
}

void InterfacesWindow::on_buttonApply_clicked() {
  InterfaceManager::get().save();
}
