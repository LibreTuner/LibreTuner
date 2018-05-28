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

#include "tuneeditor.h"
#include "definitions/definition.h"
#include "definitions/tabledefinitions.h"
#include "graphwidget.h"
#include "table.h"
#include "tablegroup.h"
#include "tune.h"
#include "ui_tuneeditor.h"

#include <cassert>
#include <utility>
#include <vector>

#include <QCloseEvent>
#include <QMessageBox>
#include <QTreeWidget>

TuneEditor::TuneEditor(TuneDataPtr tune, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::TuneEditor), tune_(tune) {
  assert(tune);

  ui->setupUi(this);
  ui->tableEdit->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  ui->labelAxisX->setVisible(false);
  ui->labelAxisY->setVisible(false);

  connect(this, &TuneEditor::tableChanged, ui->graphWidget,
          &GraphWidget::tableChanged);

  std::vector<std::pair<TableCategory, QTreeWidgetItem *>> categories_;

  TableDefinitions *tables = tune_->romData()->definition()->tables();

  for (int i = 0; i < tables->count(); ++i) {
    const TableDefinition *def = tables->at(i);
    if (!def->valid()) {
      continue;
    }

    QTreeWidgetItem *par = nullptr;

    for (auto &cat : categories_) {
      if (cat.first == def->category()) {
        par = cat.second;
        break;
      }
    }

    if (par == nullptr) {
      par = new QTreeWidgetItem(ui->treeTables);
      switch (def->category()) {
      case TCAT_LIMITER:
        par->setText(0, tr("Limits"));
        break;
      case TCAT_MISC:
        par->setText(0, tr("Miscellaneous"));
        break;
      }

      categories_.push_back(std::make_pair(def->category(), par));
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(par);
    item->setText(0, QString::fromStdString(def->name()));
    item->setData(0, Qt::UserRole, QVariant(i));
  }

  setWindowFlag(Qt::Window);
}

void TuneEditor::on_treeTables_itemActivated(QTreeWidgetItem *item,
                                             int column) {
  QVariant data = item->data(0, Qt::UserRole);
  bool ok;
  int index = data.toInt(&ok);
  if (!ok) {
    return;
  }

  if (currentTable_) {
    disconnect(currentTable_.get(), &Table::onModified, this,
               &TuneEditor::onTableModified);
  }

  currentTable_ = tune_->tables()->get(index);
  if (!currentTable_) {
    return;
  }
  connect(currentTable_.get(), &Table::onModified, this,
          &TuneEditor::onTableModified);

  ui->tableEdit->setModel(currentTable_.get());

  // This is not elegant. Maybe the class structure should be changed
  ui->labelMemory->setText(
      QStringLiteral("0x") +
      QString::number(tune_->romData()->subDefinition()->getTableLocation(
                          currentTable_->definition()->id()),
                      16));
  ui->infoName->setText(
      QString::fromStdString(currentTable_->definition()->name()));
  ui->infoDesc->setText(
      QString::fromStdString(currentTable_->definition()->description()));

  const TableAxis *axis = currentTable_->definition()->axisX();
  if (axis != nullptr) {
    ui->labelAxisX->setText(QString::fromStdString(axis->label()));
    ui->labelAxisX->setVisible(true);
    ui->tableEdit->horizontalHeader()->setVisible(true);
  } else {
    ui->labelAxisX->setVisible(false);
    ui->tableEdit->horizontalHeader()->setVisible(false);
  }

  axis = currentTable_->definition()->axisY();
  if (axis != nullptr) {
    ui->labelAxisY->setText(QString::fromStdString(axis->label()));
    ui->labelAxisY->setVisible(true);
    ui->tableEdit->verticalHeader()->setVisible(true);
  } else {
    ui->labelAxisY->setVisible(false);
    ui->tableEdit->verticalHeader()->setVisible(false);
  }

  emit tableChanged(currentTable_);
}

void TuneEditor::onTableModified() {
  if (!changed_) {
    changed_ = true;
    setWindowTitle("LibreTuner - Tune Editor *");
  }
}

void TuneEditor::closeEvent(QCloseEvent *event) {
  if (changed_) {
    QMessageBox mb;
    mb.setText(tr("This tune has been modified"));
    mb.setWindowTitle(tr("Unsaved changes"));
    mb.setInformativeText(tr("Do you want to save your changes?"));
    mb.setIcon(QMessageBox::Question);
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Discard |
                          QMessageBox::Save);
    mb.setDefaultButton(QMessageBox::Save);
    switch (mb.exec()) {
    case QMessageBox::Save:
      // Save then accept
      if (save()) {
        event->accept();
        return;
      }
      event->ignore();
      break;
    case QMessageBox::Discard:
      event->accept();
      break;
    case QMessageBox::Cancel:
    default:
      event->ignore();
    }
  }
}

bool TuneEditor::save() {
  if (tune_->save()) {
    return true;
  }

  QMessageBox mb;
  mb.setText(tr("Failed to save tune data"));
  mb.setWindowTitle(tr("Failed to save"));
  mb.setInformativeText(QString::fromStdString(tune_->lastError()));
  mb.setIcon(QMessageBox::Critical);
  mb.setStandardButtons(QMessageBox::Ok);
  mb.exec();

  return false;
}
