#ifndef INTERFACESWINDOW_H
#define INTERFACESWINDOW_H

#include <QListWidgetItem>
#include <QAbstractItemModel>
#include <QWidget>
#include <gsl/span>

#include "interfacemanager.h"
#include "styledwindow.h"

namespace Ui {
class InterfacesWindow;
}

class SettingsWidget;

class InterfacesModel : public QAbstractItemModel {
public:


    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
};

class InterfacesWindow : public QWidget {
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
  InterfacesModel model_;

  void replaceSettings(std::unique_ptr<SettingsWidget> widget);

  void interfacesChanged(gsl::span<const InterfaceSettingsPtr> interfaces);
};

#endif // INTERFACESWINDOW_H
