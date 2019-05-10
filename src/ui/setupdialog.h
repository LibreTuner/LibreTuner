//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_DEFINITIONVIEW_H
#define LIBRETUNER_DEFINITIONVIEW_H

#include "lt/definition/platform.h"
#include <QDialog>

class QAbstractItemModel;
class QComboBox;

class SetupDialog : public QDialog {
public:
    explicit SetupDialog(QWidget *parent = nullptr);

    // Sets the definitions model
    void setDefinitionModel(QAbstractItemModel *model);

    // Sets the datalinks model
    void setDatalinksModel(QAbstractItemModel *model);

    // Returns the selected platform or nullptr if none are selected
    lt::PlatformPtr platform();

    // Returns the selected datalink or nullptr if none are selected
    lt::DataLink *datalink();

private:
    QComboBox *comboPlatforms_;
    QComboBox *comboDatalinks_;
};

#endif // LIBRETUNER_DEFINITIONVIEW_H
