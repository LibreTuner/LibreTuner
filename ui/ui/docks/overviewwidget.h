#ifndef OVERVIEWWIDGET_H
#define OVERVIEWWIDGET_H

#include <QWidget>

class QLineEdit;

/**
 * Shows an overview
 */
class OverviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverviewWidget(QWidget * parent = nullptr);

private:
    QLineEdit * lineDefinitionCount_;
    QLineEdit * lineRomCount_;
    QLineEdit * lineDatalinkCount_;
};

#endif // OVERVIEWWIDGET_H
