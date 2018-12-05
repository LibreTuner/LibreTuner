#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H


#include <QWidget>
#include <QToolButton>
#include <QTreeWidget>

class SidebarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

private slots:
    void on_treeToolButton_clicked(bool checked);

private:
    QToolButton *tableInfoButton_;
    QTreeWidget *tableTreeWidget_;
};

#endif // SIDEBARWIDGET_H
