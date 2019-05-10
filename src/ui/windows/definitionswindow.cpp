#include "definitionswindow.h"

#include "database/definitions.h"

#include <QHBoxLayout>

DefinitionsWindow::DefinitionsWindow(QWidget *parent) : QWidget(parent) {
    platformsView_ = new QTreeView;

    auto *layout = new QHBoxLayout;
    layout->addWidget(platformsView_);
    setLayout(layout);
}

void DefinitionsWindow::setDefinitions(Definitions *definitions) noexcept {
    definitions_ = definitions;
    platformsView_->setModel(definitions);
}
