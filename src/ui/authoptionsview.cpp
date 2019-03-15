#include "authoptionsview.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>

AuthOptionsView::AuthOptionsView(QWidget *parent) : QWidget(parent)
{

    // Options
    lineKey_ = new QLineEdit;
    
    spinSessionId_ = new QSpinBox;
    spinSessionId_->setRange(0, 0xFF);
    spinSessionId_->setDisplayIntegerBase(16);
    spinSessionId_->setPrefix("0x");
    
    // Form
    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    form->setLabelAlignment(Qt::AlignTop);
    form->setContentsMargins(0, 0, 0, 0);
    form->addRow(tr("Key"), lineKey_);
    form->addRow(tr("Session ID"), spinSessionId_);
    
    // Buttons
    auto *buttonReset = new QPushButton(tr("Reset to Default"));
    
    // Main layout
    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(form);
    layout->addWidget(buttonReset);

    setLayout(layout);
    
    connect(buttonReset, &QPushButton::clicked, this, &AuthOptionsView::resetOptions);
}



void AuthOptionsView::setDefaultOptions(const lt::auth::Options& options)
{
    defaultOptions_ = options;
    resetOptions();
}



void AuthOptionsView::resetOptions()
{
    lineKey_->setText(QString::fromStdString(defaultOptions_.key));
    spinSessionId_->setValue(defaultOptions_.session);
}

