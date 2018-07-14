#ifndef INTERFACESWINDOW_H
#define INTERFACESWINDOW_H

#include <QListWidgetItem>
#include <QWidget>
#include <gsl/span>

#include "interfacemanager.h"
#include "styledwindow.h"

namespace Ui {
class InterfacesWindow;
}

class SettingsWidget;

class InterfacesWindow : public StyledWindow {
  Q_OBJECT

public:
  explicit InterfacesWindow(QWidget *parent = nullptr);
  ~InterfacesWindow() override;

private slots:
  void on_buttonAdd_clicked();
  void on_buttonRemove_clicked();
  void on_buttonApply_clicked();

  void on_listInterfaces_currentItemChanged(QListWidgetItem *current,
                                            QListWidgetItem *previous);

private:
  Ui::InterfacesWindow *ui;
  std::shared_ptr<InterfaceManager::ConnType> conn_;
  std::unique_ptr<SettingsWidget> settings_;

  void replaceSettings(std::unique_ptr<SettingsWidget> widget);

  void interfacesChanged(gsl::span<const InterfaceSettingsPtr> interfaces);
};

#endif // INTERFACESWINDOW_H
