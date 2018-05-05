#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QItemModelSurfaceDataProxy>
#include <QSurface3DSeries>
#include <Q3DSurface>
#include <QtCharts>

#include <memory>

class Table;
typedef std::shared_ptr<Table> TablePtr;

/**
 * @todo write docs
 */
class GraphWidget : public QWidget
{
public:
    explicit GraphWidget(QWidget *parent = 0);
    
public slots:
    void tableChanged(TablePtr table);
    
private:
    TablePtr table_;
    QtDataVisualization::Q3DSurface *surface_;
    QWidget *container_;
    QtCharts::QChart *chart_;
    QtCharts::QChartView *chartView_;
    
    QtDataVisualization::QSurface3DSeries series3d_;
};

#endif // GRAPHWIDGET_H
