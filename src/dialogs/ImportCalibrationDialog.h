#ifndef IMPORTCALIBRATION_H
#define IMPORTCALIBRATION_H

#include <QWizard>

#include <models/PlatformsModel.h>

namespace Ui
{
class ImportCalibrationDialog;
}

class ImportCalibrationDialog : public QWizard
{
    Q_OBJECT

public:
    explicit ImportCalibrationDialog(QString path, QWidget * parent = nullptr);
    ~ImportCalibrationDialog() override;

    // Returns selected platform
    const lt::Platform * selectedPlatform() const;

private:
    Ui::ImportCalibrationDialog * ui;

    QString path_;
    PlatformsModel platformsModel_;
};

#endif // IMPORTCALIBRATION_H
