#ifndef DIAGNOSTICSWIDGET_H
#define DIAGNOSTICSWIDGET_H

#include <QWidget>
#include <QListView>

#include "scanresult.h"

class DiagnosticsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DiagnosticsWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    QListView *listCodes_;

    ScanResult scanResult_;
};

#endif // DIAGNOSTICSWIDGET_H
