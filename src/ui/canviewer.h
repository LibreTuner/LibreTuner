#ifndef CANVIEWER_H
#define CANVIEWER_H

#include <QWidget>

namespace Ui {
class CanViewer;
}

class CanHandler;
class QAbstractItemModel;

class CanViewer : public QWidget
{
    Q_OBJECT

public:
    explicit CanViewer(QWidget *parent = 0);
    ~CanViewer();

private:
    Ui::CanViewer *ui;
    QAbstractItemModel *logModel_ = nullptr;
    
public slots:
    void rowsInserted(const QModelIndex &parent, int first, int last);
};

#endif // CANVIEWER_H
