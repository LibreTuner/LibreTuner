#ifndef DIAGNOSTICSWIDGET_H
#define DIAGNOSTICSWIDGET_H

#include <QTableView>
#include <QWidget>

#include "scanresult.h"

class QCheckBox;

class DiagnosticsWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiagnosticsWidget(QWidget *parent = nullptr);

signals:

public slots:
    void scan();
    void clear();

private:
    QTableView *listCodes_;
    QCheckBox *checkPending_;

    ScanResult scanResult_;
    ScanResult pendingScanResult_;
};

#endif // DIAGNOSTICSWIDGET_H
