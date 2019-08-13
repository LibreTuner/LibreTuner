#include "scalarview.h"

#include <lt/rom/table.h>

#include <QCloseEvent>
#include <QDoubleValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include <uiutil.h>

void ScalarView::setTable(lt::Table * table)
{
    table_ = table;
    if (table_ == nullptr)
        return;
    setWindowTitle(QString::fromStdString(table->name()));
    description_->setText(QString::fromStdString(table->description()));
    slider_->setMinimum(static_cast<int>(table_->minimum() * 1000.0));
    slider_->setMaximum(static_cast<int>(table_->maximum() * 1000.0));
    setValue(table_->get(0, 0));
}

ScalarView::ScalarView(QWidget * parent) : QWidget(parent)
{
    lineValue_ = new QLineEdit;
    lineValue_->setValidator(new QDoubleValidator);

    slider_ = new QSlider;
    slider_->setTracking(true);
    slider_->setOrientation(Qt::Horizontal);

    description_ = new QLabel;
    description_->setWordWrap(true);
    description_->setAlignment(Qt::AlignCenter);

    buttonResetBase_ = new QPushButton(tr("Reset to Base"));
    buttonReset_ = new QPushButton(tr("Reset"));
    buttonSave_ = new QPushButton(tr("Save"));

    auto * layoutButton = new QHBoxLayout;
    layoutButton->addWidget(buttonSave_);
    layoutButton->addWidget(buttonReset_);
    layoutButton->addStretch();
    layoutButton->addWidget(buttonResetBase_);

    auto * layout = new QVBoxLayout;
    layout->addWidget(description_);
    layout->addWidget(lineValue_);
    layout->addWidget(slider_);
    layout->addLayout(layoutButton);
    setLayout(layout);

    connect(slider_, &QSlider::sliderMoved, [this](int value) {
        double scaled = static_cast<double>(value) / 1000.0;
        lineValue_->setText(QString::number(scaled));
        setDirty(true);
    });

    connect(lineValue_, &QLineEdit::textEdited, [this](const QString & text) {
        bool ok;
        double value = text.toDouble(&ok);
        if (!ok)
            return;
        slider_->setValue(static_cast<int>(value * 1000.0));
        setDirty(true);
    });

    connect(buttonResetBase_, &QPushButton::clicked, [this]() {
        if (table_ == nullptr)
            return;
        catchWarning(
            [&]() {
                setValue(table_->getBase(0, 0));
                setDirty(true);
            },
            tr("Error resetting scalar"));
    });

    connect(buttonReset_, &QPushButton::clicked, [this]() {
        if (table_ == nullptr)
            return;
        catchWarning(
            [&]() {
                setValue(table_->get(0, 0));
                setDirty(false);
            },
            tr("Error resetting scalar"));
    });

    connect(buttonSave_, &QPushButton::clicked, [this]() { save(); });
}

void ScalarView::setValue(double value)
{
    lineValue_->setText(QString::number(value));
    slider_->setValue(static_cast<int>(value * 1000.0));
}

void ScalarView::setDirty(bool dirty)
{
    if (table_ == nullptr)
        return;
    dirty_ = dirty;
    if (dirty_)
        setWindowTitle(QString::fromStdString(table_->name()) + " *");
    else
        setWindowTitle(QString::fromStdString(table_->name()));
}

void ScalarView::closeEvent(QCloseEvent * event)
{
    if (!dirty_)
        return;

    int res = QMessageBox::question(nullptr, tr("Unsaved changed"), tr("Save changes before closing?"),
                                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (res == QMessageBox::Yes)
    {
        save();
        event->accept();
        return;
    }
    if (res == QMessageBox::No)
    {
        event->accept();
        return;
    }
    if (res == QMessageBox::Cancel)
    {
        event->ignore();
    }
}

void ScalarView::save()
{
    if (table_ == nullptr)
        return;
    bool ok;
    double value = lineValue_->text().toDouble(&ok);
    if (!ok)
        return;
    catchWarning(
        [&]() {
            table_->set(0, 0, value);
            setDirty(false);
        },
        tr("Error saving scalar"));
}
