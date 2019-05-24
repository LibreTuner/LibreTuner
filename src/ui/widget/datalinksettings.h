#ifndef LIBRETUNER_DATALINKSETTINGS_H
#define LIBRETUNER_DATALINKSETTINGS_H

#include <QWidget>

#include "lt/link/datalink.h"

class DataLinkSettings : public QWidget {
    Q_OBJECT
public:
    DataLinkSettings(lt::DataLinkFlags flags, QWidget *parent = nullptr);
private:
};



#endif //LIBRETUNER_DATALINKSETTINGS_H
