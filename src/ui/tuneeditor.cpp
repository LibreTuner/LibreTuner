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

TuneEditor::TuneEditor(const std::shared_ptr<Tune> &tune, QWidget *parent)
    : StyledWindow(parent), ui(new Ui::TuneEditor), tune_(tune) {
    assert(tune);
    setTitle("LibreTuner - Tune Editor");

    QMainWindow *wrapper = new QMainWindow;
    layout_->addWidget(wrapper);

    ui->setupUi(wrapper);
    resize(1050, 500);
    ui->tableEdit->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->labelAxisX->setVisible(false);
    ui->labelAxisY->setVisible(false);


    ui->tableEdit->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->tableEdit->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);

    connect(this, &TuneEditor::tableChanged, ui->graphWidget,
            &GraphWidget::tableChanged);

    connect(ui->treeTables, &QTreeWidget::itemActivated, this,
            &TuneEditor::on_treeTables_itemActivated);

    std::vector<std::pair<std::string, QTreeWidgetItem *>> categories_;

    for (const definition::Table &def : tune_->rom()->definition()->main.tables) {
        QTreeWidgetItem *par = nullptr;

        for (auto &cat : categories_) {
            if (cat.first == def.category) {
                par = cat.second;
                break;
            }
        }

        if (par == nullptr) {
            par = new QTreeWidgetItem(ui->treeTables);
            par->setText(0, QString::fromStdString(def.category));

            categories_.emplace_back(def.category, par);
        }

        auto *item = new QTreeWidgetItem(par);
        item->setText(0, QString::fromStdString(def.name));
        item->setData(0, Qt::UserRole, QVariant(def.id));
    }

    setWindowFlag(Qt::Window);
}



void TuneEditor::on_treeTables_itemActivated(QTreeWidgetItem *item,
                                             int /*column*/) {
    QVariant data = item->data(0, Qt::UserRole);
    bool ok;
    int index = data.toInt(&ok);
    if (!ok) {
        return;
    }

    if (currentTable_.table) {
        //disconnect(currentTable_.get(), &Table::onModified, this,
        //           &TuneEditor::onTableModified);
    }

    currentTable_.table = tune_->tables().get(index);
    if (!currentTable_.table) {
        return;
    }
    currentTable_.id = index;
    //connect(currentTable_, &Table::onModified, this,
    //        &TuneEditor::onTableModified);

    ui->tableEdit->setModel(currentTable_.table);

    // This is not elegant. Maybe the class structure should be changed
    ui->labelMemory->setText(
        QStringLiteral("0x") +
        QString::number(tune_->rom()->definition()->tables[
                            currentTable_.id],
                        16));
    ui->infoName->setText(
        QString::fromStdString(currentTable_.table->name()));
    ui->infoDesc->setText(
        QString::fromStdString(currentTable_.table->description()));

    /*const TableAxis *axis = currentTable_.table->definition()->axisX();
    if (axis != nullptr) {
        ui->labelAxisX->setText(QString::fromStdString(axis->label()));
        ui->labelAxisX->setVisible(true);
        ui->tableEdit->horizontalHeader()->setVisible(true);
    } else {
        ui->labelAxisX->setVisible(false);
        ui->tableEdit->horizontalHeader()->setVisible(false);
    }

    axis = currentTable_.table->definition()->axisY();
    if (axis != nullptr) {
        ui->labelAxisY->setText(QString::fromStdString(axis->label()));
        ui->labelAxisY->setVisible(true);
        ui->tableEdit->verticalHeader()->setVisible(true);
    } else {
        ui->labelAxisY->setVisible(false);
        ui->tableEdit->verticalHeader()->setVisible(false);
    }*/

    emit tableChanged(currentTable_.table);
}



void TuneEditor::onTableModified() {
    if (!changed_) {
        changed_ = true;
        setTitle("LibreTuner - Tune Editor *");
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
    try {
        tune_->save();
        return true;
    } catch (const std::exception &e) {
        QMessageBox mb;
        mb.setText(tr("Failed to save tune data"));
        mb.setWindowTitle(tr("Failed to save"));
        mb.setInformativeText(e.what());
        mb.setIcon(QMessageBox::Critical);
        mb.setStandardButtons(QMessageBox::Ok);
        mb.exec();

        return false;
    }
}
