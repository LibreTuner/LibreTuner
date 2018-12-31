#include "platformlinkview.h"
#include "definitions/definitionmanager.h"
#include "interfacemanager.h"

#include <QFormLayout>


Q_DECLARE_METATYPE(InterfaceSettings*)

PlatformLinkView::PlatformLinkView(QWidget* parent) : QWidget(parent)
{
    auto *formLayout = new QFormLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    
    comboDatalink_ = new QComboBox;
    comboPlatform_ = new QComboBox;
    
    formLayout->addRow("Datalink", comboDatalink_);
    formLayout->addRow("Platform", comboPlatform_);
    
    
    setLayout(formLayout);
}

Q_DECLARE_METATYPE(definition::MainPtr)
Q_DECLARE_METATYPE(datalink::Link*)

std::unique_ptr<VehicleLink> PlatformLinkView::getLink()
{
    QVariant datalinkVar = comboDatalink_->currentData(Qt::UserRole);
    if (!datalinkVar.canConvert<datalink::Link*>()) {
        return nullptr;
    }
    //InterfaceSettings *iSettings  = datalinkVar.value<InterfaceSettings*>();
    //if (!iSettings) {
    //    return nullptr;
    //}
    //DataLinkPtr datalink = DataLink::create(*iSettings);

    auto *datalink = datalinkVar.value<datalink::Link*>();
    
    // Get platform
    
    QVariant platformVar = comboPlatform_->currentData(Qt::UserRole);
    if (!platformVar.canConvert<definition::MainPtr>()) {
        return nullptr;
    }
    
    auto def = platformVar.value<definition::MainPtr>();
    
    //return std::make_unique<VehicleLink>(std::move(def), std::move(datalink));
    return nullptr;
}



void PlatformLinkView::setDatalinkModel(QAbstractItemModel *model) {
    comboDatalink_->setModel(model);
}



void PlatformLinkView::setPlatformModel(QAbstractItemModel *model) {
    comboPlatform_->setModel(model);
}
