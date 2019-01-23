#ifndef PLATFORMLINKVIEW_H
#define PLATFORMLINKVIEW_H

#include <QWidget>
#include <QComboBox>

#include "vehicle.h"

/**
 * @todo write docs
 */
class PlatformLinkView : public QWidget
{
public:
    explicit PlatformLinkView(QWidget *parent = nullptr);
    
    std::unique_ptr<PlatformLink> getLink();

    void setDatalinkModel(QAbstractItemModel *model);
    void setPlatformModel(QAbstractItemModel *model);
    
private:
    QComboBox *comboDatalink_;
    QComboBox *comboPlatform_;
};

#endif // PLATFORMLINKVIEW_H
