#include "editorwidget.h"

#include "rom.h"
#include "libretuner.h"

#include "verticallabel.h"

#include <QTableView>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>

EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;

    labelX_ = new QLabel("X-Axis");
    labelX_->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelX_);

    QHBoxLayout *hLayout = new QHBoxLayout;

    view_ = new QTableView(this);
    view_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    view_->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    view_->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    
    view_->horizontalHeader()->setDefaultSectionSize(70);
    view_->horizontalHeader()->setMinimumSectionSize(70);
    view_->horizontalHeader()->setStretchLastSection(true);


    labelY_ = new VerticalLabel("Y-Axis");
    labelY_->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(labelY_);
    hLayout->addWidget(view_);

    layout->addLayout(hLayout);
    setLayout(layout);
}



void EditorWidget::setModel(QAbstractItemModel *model)
{
    view_->setModel(model);
}



void EditorWidget::tableChanged(Table *table)
{
    setModel(table);
    
    if (!table) {
        return;
    }
    
    if (table->axisX()) {
        labelX_->setText(QString::fromStdString(table->axisX()->name()));
        labelX_->setVisible(true);
        view_->horizontalHeader()->setVisible(true);
    } else {
        labelX_->setVisible(false);
        view_->horizontalHeader()->setVisible(false);
    }
    
    if (table->axisY()) {
        labelY_->setText(QString::fromStdString(table->axisY()->name()));
        labelY_->setVisible(true);
        view_->verticalHeader()->setVisible(true);
    } else {
        labelY_->setVisible(false);
        view_->verticalHeader()->setVisible(false);
    }
    
    // This is not elegant. Maybe the class structure should be changed
    /*ui->labelMemory->setText(
        QStringLiteral("0x") +
        QString::number(tune_->rom()->definition()->tables[
                            currentTable_.id],
                        16));
    ui->infoName->setText(
        QString::fromStdString(currentTable_.table->name()));
    ui->infoDesc->setText(
        QString::fromStdString(currentTable_.table->description()));*/

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

    //emit tableChanged(currentTable_.table);
}



void EditorWidget::tuneChanged(const std::shared_ptr<Tune>& tune)
{
    tune_ = tune;
    tuneData_.reset();
}

