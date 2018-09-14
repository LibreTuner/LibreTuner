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

#include "styledwindow.h"

namespace Ui {
class TuneEditor;
}

class Tune;

class Table;
using TablePtr = std::shared_ptr<Table>;


struct EditTable {
    Table *table {nullptr};
    std::size_t id;
};

/**
 * Tune editor window
 */
class TuneEditor : public StyledWindow {
    Q_OBJECT
public:
    TuneEditor(const std::shared_ptr<Tune> &tune, QWidget *parent = 0);

    void closeEvent(QCloseEvent *event) override;

    bool save();

private:
    std::shared_ptr<Tune> tune_;
    Ui::TuneEditor *ui;
    EditTable currentTable_;

    /* Set to true if a table has been modified */
    bool changed_ = false;

private slots:
    void on_treeTables_itemActivated(QTreeWidgetItem *item, int column);
    void onTableModified();

signals:
    void tableChanged(Table *table);
};

#endif // TUNEEDITOR_H
