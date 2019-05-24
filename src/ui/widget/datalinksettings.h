#ifndef LIBRETUNER_DATALINKSETTINGS_H
#define LIBRETUNER_DATALINKSETTINGS_H

#include <QWidget>

#include "lt/link/datalink.h"

class QLineEdit;
class QSpinBox;
class CustomCombo;
class QCheckBox;
class QLabel;
class QHBoxLayout;
class SerialPortModel;

namespace lt {
class DataLink;
}

class DataLinkSettings : public QWidget {
    Q_OBJECT
public:
    explicit DataLinkSettings(lt::DataLinkFlags flags = lt::DataLinkFlags::None, QWidget *parent = nullptr);

    void setFlags(lt::DataLinkFlags flags);
    void setPortType(lt::DataLinkPortType type);
    void apply(lt::DataLink &link);

    void reset();
    // Fill form from datalink. Resets if nullptr.
    void fill(lt::DataLink *link);

    QString name() const;

    void setPort(const QString &port);
    QString port() const;
    int baudrate() const;

signals:
    void settingChanged();

private:

    QLineEdit *lineName_;
    CustomCombo *comboPort_{nullptr};
    QSpinBox *spinBaudrate_{nullptr};
    QCheckBox *checkBaudrate_{nullptr};

    QLabel *labelPort_;
    QLabel *labelBaudrate_;
    QHBoxLayout *baudrateLayout_;

    SerialPortModel *portModel_;
};



#endif //LIBRETUNER_DATALINKSETTINGS_H
