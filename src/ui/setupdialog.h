//
// Created by altenius on 12/21/18.
//

#ifndef LIBRETUNER_DEFINITIONVIEW_H
#define LIBRETUNER_DEFINITIONVIEW_H

#include <QDialog>
#include "datalink/datalink.h"
#include "definitions/definition.h"

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
    definition::MainPtr platform();

    // Returns the selected datalink or nullptr if none are selected
    datalink::Link *datalink();

private:
    QComboBox *comboPlatforms_;
    QComboBox *comboDatalinks_;
};


#endif //LIBRETUNER_DEFINITIONVIEW_H
