/*
 * LibreTuner
 * Copyright (C) 2018 Altenius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TUNEEDITOR_H
#define TUNEEDITOR_H

#include <QAbstractItemModel>
#include <QMainWindow>
#include <QTreeWidgetItem>

#include <memory>

namespace Ui {
class TuneEditor;
}

class TuneData;
typedef std::shared_ptr<TuneData> TuneDataPtr;
class Table;
typedef std::shared_ptr<Table> TablePtr;

/**
 * Tune editor window
 */
class TuneEditor : public QMainWindow {
  Q_OBJECT
public:
  TuneEditor(TuneDataPtr tune, QWidget *parent = 0);

  void closeEvent(QCloseEvent *event) override;

  bool save();

private:
  TuneDataPtr tune_;
  Ui::TuneEditor *ui;
  TablePtr currentTable_;

  /* Set to true if a table has been modified */
  bool changed_ = false;

private slots:
  void on_treeTables_itemActivated(QTreeWidgetItem *item, int column);
  void onTableModified();

signals:
  void tableChanged(TablePtr table);
};

#endif // TUNEEDITOR_H
