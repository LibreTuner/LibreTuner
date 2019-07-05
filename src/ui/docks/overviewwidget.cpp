#include "overviewwidget.h"

#include "libretuner.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

OverviewWidget::OverviewWidget(QWidget *parent) : QWidget(parent) {
    lineDefinitionCount_ = new QLineEdit;
    lineDefinitionCount_->setReadOnly(true);

    lineRomCount_ = new QLineEdit;
    lineRomCount_->setReadOnly(true);

    lineDatalinkCount_ = new QLineEdit;
    lineDatalinkCount_->setReadOnly(true);

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Definitions loaded:"), lineDefinitionCount_);
    form->addRow(tr("ROMs loaded:"), lineRomCount_);
    form->addRow(tr("Datalinks detected:"), lineDatalinkCount_);

    lineDefinitionCount_->setText(QString::number(LT()->definitions().size()));
    lineDatalinkCount_->setText(QString::number(LT()->links().count()));

    auto *layout = new QVBoxLayout;
    layout->addLayout(form);

    setLayout(layout);
}
