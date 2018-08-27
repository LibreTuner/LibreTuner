#include "diagnosticswidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>

DiagnosticsWidget::DiagnosticsWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout();
    QPushButton *buttonScan = new QPushButton(tr("Scan for diagnostic codes"));
    layout->addWidget(buttonScan);
    connect(buttonScan, &QPushButton::clicked, [] {
        // Stub
    });

    listCodes_ = new QListView;
    listCodes_->setModel(&scanResult_);
    layout->addWidget(listCodes_);

    setLayout(layout);
}
