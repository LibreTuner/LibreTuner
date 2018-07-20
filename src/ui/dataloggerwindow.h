#ifndef DATALOGGERWINDOW_H
#define DATALOGGERWINDOW_H

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include "vehicle.h"

#include <memory>

class QListWidget;

class DataLink;
using DataLinkPtr = std::shared_ptr<DataLink>;

class DataLogger;
using DataLoggerPtr = std::shared_ptr<DataLogger>;

class DataLog;
using DataLogPtr = std::shared_ptr<DataLog>;

class DataLoggerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DataLoggerWindow(QWidget *parent = nullptr);

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

signals:

public slots:
    /* Called when the vehicle definition has been queried */
    void queried();

    /* Callback for the start/stop button */
    void buttonClicked();

private:
    DataLogPtr log_;
    DataLoggerPtr logger_;
    QListWidget *pidList_;
    QTreeWidget *logOutput_;
    QPushButton *buttonLog_;

    DataLinkPtr link_;
    DefinitionPtr definition_;
    VehiclePtr vehicle_;
};

#endif // DATALOGGERWINDOW_H
