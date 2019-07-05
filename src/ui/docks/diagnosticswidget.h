#ifndef DIAGNOSTICSWIDGET_H
#define DIAGNOSTICSWIDGET_H

#include <QTableView>
#include <QWidget>

#include "lt/diagnostics/codes.h"
#include "models/dtcmodel.h"

class QCheckBox;

class DiagnosticsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DiagnosticsWidget(QWidget * parent = nullptr);

signals:

public slots:
    void scan();
    void clear();

private:
    QTableView * listCodes_;
    QCheckBox * checkPending_;

    DtcModel dtcModel_;
    DtcModel pendingDtcModel_;
};

#endif // DIAGNOSTICSWIDGET_H
