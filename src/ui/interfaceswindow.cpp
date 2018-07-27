#include "interfaceswindow.h"
#include "ui_interfaceswindow.h"

#include "addinterfacedialog.h"
#include "settingswidget.h"

#include <QMessageBox>

Q_DECLARE_METATYPE(InterfaceSettingsPtr);

InterfacesWindow::InterfacesWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::InterfacesWindow) {
  ui->setupUi(this);
  ui->listInterfaces->setModel(&model_);
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
  QListWidgetItem *item = nullptr;//ui->listInterfaces->currentItem();
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
  //ui->listInterfaces->clear();

  for (const auto &iface : interfaces) {
    //QListWidgetItem *item =
    //    new QListWidgetItem(QString::fromStdString(iface->name()));
    //item->setData(Qt::UserRole,
    //              QVariant::fromValue<InterfaceSettingsPtr>(iface));
    //ui->listInterfaces->addItem(item);
  }
}

void InterfacesWindow::on_listInterfaces_currentItemChanged(
    QListWidgetItem *current, QListWidgetItem * /*previous*/) {
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

Qt::ItemFlags InterfacesModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex InterfacesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        // We store the type index as the pointer
        return createIndex(row, column, reinterpret_cast<void*>(parent.row() + 1));
    }

    if (row > 1 || column != 0) {
        return QModelIndex();
    }
    return createIndex(row, column, reinterpret_cast<void*>(0));
}

QModelIndex InterfacesModel::parent(const QModelIndex &child) const
{
    int ind = reinterpret_cast<int>(child.internalPointer());
    if (ind != 0) {
        return createIndex(ind - 1, 0, nullptr);
    }
    return QModelIndex();
}

#include <iostream>
int InterfacesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        // Root
        return 2;
    }
    QModelIndex p = parent.parent();
    if (p.isValid()) {
        int row = p.row();
        if (row == 0) {
            // Manual
            return InterfaceManager::get().settings().size();
        } else if (row == 1) {
            // Auto-detect
            return 0;
        }
        return 0;
    }

    return 0;
}

int InterfacesModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant InterfacesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (index.column() != 0) {
        return QVariant();
    }

    QModelIndex parent = index.parent();
    if (!parent.isValid()) {
        int row = index.row();
        if (row == 0) {
            return "Manually added";
        } else if (row == 1) {
            return "Autodetected";
        }
        return QVariant();
    }
    if (parent.row() == 0) {
        auto settings = InterfaceManager::get().settings();
        if (settings.size() > index.row()) {
            return QString::fromStdString(settings.at(index.row())->name());
        }
    }
    return QVariant();
}
