#ifndef LIBRETUNER_VEHICLEINFORMATIONWIDGET_H
#define LIBRETUNER_VEHICLEINFORMATIONWIDGET_H

#include <QWidget>

class QLineEdit;

class VehicleInformationWidget : public QWidget {
public:
    explicit VehicleInformationWidget(QWidget * parent = nullptr);

public slots:
    void scan();

private:
    QLineEdit * lineVin_;
    QLineEdit * lineCalibrationId_;
    QLineEdit * lineEcuName_;
};

#endif // LIBRETUNER_VEHICLEINFORMATIONWIDGET_H
