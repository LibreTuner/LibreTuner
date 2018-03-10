#ifndef CANLOGVIEW_H
#define CANLOGVIEW_H

#include <QTableView>

/**
 * @todo write docs
 */
class CanLogView : public QTableView
{
    Q_OBJECT
public:
    explicit CanLogView(QWidget *parent = 0);
};

#endif // CANLOGVIEW_H
