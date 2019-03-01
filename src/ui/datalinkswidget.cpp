//
// Created by altenius on 12/21/18.
//

#include "datalinkswidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTreeView>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>

#include "datalink/linkdatabase.h"
#include "libretuner.h"



int DataLinksTreeModel::rowCount(const QModelIndex& parent) const {
    if (!parent.isValid()) {
        return 2;
    }
    
    if (!links_) {
        return 0;
    }
    
    if (parent.internalId() != 0) {
        // Link-level
        return 0;
    }
    
    switch (parent.row()) {
        case 0:
            return links_->detectedCount();
        case 1:
            return links_->manualCount();
        default:
            return 0;
    }
}



int DataLinksTreeModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}



QVariant DataLinksTreeModel::data(const QModelIndex& index, int role) const {
    if (!links_) {
        return QVariant();
    }
    
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    
    if (!index.isValid()) {
        return QVariant();
    }
    
    if (index.internalId() == 0) {
        if (index.column() != 0) {
            return QVariant();
        }
        if (index.row() == 0) {
            return tr("Auto-detected");
        } else if (index.row() == 1) {
            return tr("Manual");
        }
        return QVariant();
    }
    
    datalink::Link *link{nullptr};
    
    if (index.internalId() == 1) {
        // Auto-detected
        link = links_->getDetected(index.row());
    } else if (index.internalId() == 2) {
        link = links_->getManual(index.row());
    }
    if (link == nullptr) {
        return QVariant();
    }
    
    if (index.column() == 0) {
        return QString::fromStdString(link->name());
    }
    
    return QVariant();
}


QVariant DataLinksTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section < 0 || section > 1) {
        return QVariant();
    }
    
    if (section == 0) {
        return tr("Name");
    } else {
        return tr("Type");
    }
}



QModelIndex DataLinksTreeModel::parent(const QModelIndex& child) const {
    if (child.isValid()) {
        if (child.internalId() != 0) {
            // link-level indicies have their internal id set to the parent row + 1
            return createIndex(child.internalId() - 1, 0, static_cast<quintptr>(0));
        }
    }
    
    return QModelIndex();
}



QModelIndex DataLinksTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!links_) {
        return QModelIndex();
    }
    
    if (parent.isValid()) {
        if (parent.internalId() == 0) {
            // Top level
            if (parent.row() == 0) {
                if (row < links_->detectedCount()) {
                    return createIndex(row, column, parent.row() + 1);
                }
            } else if (parent.row() == 1) {
                if (row < links_->manualCount()) {
                    return createIndex(row, column, parent.row() + 1);
                }
            }
        }
        return QModelIndex();
    }
    
    if (row >= 0 && row < 2) {
        return createIndex(row, column, static_cast<quintptr>(0));
    }
    
    return QModelIndex();
}


Qt::ItemFlags DataLinksTreeModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}



DatalinksWidget::DatalinksWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("LibreTuner - Datalinks"));
    resize(600, 400);
    
    auto *buttonAdd = new QPushButton(tr("Add"));
    auto *buttonRemove = new QPushButton(tr("Remove"));
    
    auto *lineName = new QLineEdit;
    auto *linePort = new QLineEdit;

    auto *linksView = new QTreeView;
    linksView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    linksModel_.setLinks(&LT()->links());
    linksView->setModel(&linksModel_);
    
    // Layouts
    auto *buttonLayout = new QVBoxLayout;
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->addWidget( buttonAdd );
    buttonLayout->addWidget( buttonRemove );

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name"), lineName);
    formLayout->addRow(tr("Port"), linePort);
    
    auto *linksLayout = new QVBoxLayout;
    linksLayout->addWidget(linksView);
    linksLayout->addLayout(formLayout);

    auto *layout = new QHBoxLayout;
    layout->addLayout(linksLayout);
    layout->addLayout(buttonLayout);
    setLayout(layout);
}
