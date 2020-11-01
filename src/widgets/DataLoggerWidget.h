#ifndef DATALOGGERWIDGET_H
#define DATALOGGERWIDGET_H

#include <QWidget>

namespace Ui {
class DataLoggerWidget;
}

class DataLoggerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataLoggerWidget(QWidget *parent = nullptr);
    ~DataLoggerWidget();

private:
    Ui::DataLoggerWidget *ui;
};

#endif // DATALOGGERWIDGET_H
