#include "sessionscannerdialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QListView>

SessionScannerDialog::SessionScannerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("LibreTuner - Session Scanner"));
    auto *layout = new QVBoxLayout;

    auto *sessionList = new QListView;
    sessionList->setModel(&sessions_);

    auto *start = new QPushButton(tr("Start"));

    connect(start, &QPushButton::clicked, [this]() {
        scan();
    });

    layout->addWidget(sessionList);
    layout->addWidget(start);

    setLayout(layout);
}



void SessionScannerDialog::scan()
{

}
