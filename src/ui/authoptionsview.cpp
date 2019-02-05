#include "authoptionsview.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>

AuthOptionsView::AuthOptionsView(QWidget *parent) : QWidget(parent)
{
    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);

    auto *layoutKey = new QVBoxLayout;
    layoutKey->setContentsMargins(0, 0, 0, 0);

    auto *checkCustomKey = new QCheckBox(tr("Use custom key"));
    lineKey_ = new QLineEdit;

    layoutKey->addWidget(checkCustomKey);
    layoutKey->addWidget(lineKey_);

    form->addRow(tr("Key"), layoutKey);

    setLayout(form);
}
