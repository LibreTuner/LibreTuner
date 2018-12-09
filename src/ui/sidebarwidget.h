#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H


#include <QWidget>
#include <QToolButton>
#include <QTreeWidget>


class Table;

class QPlainTextEdit;


class SidebarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    
public slots:
    void fillTableInfo(Table *table);

private slots:
    void on_treeToolButton_clicked(bool checked);

private:
    QToolButton *tableInfoButton_;
    QTreeWidget *tableTreeWidget_;
    
    QPlainTextEdit *tableDescription_;
    
    QTreeWidgetItem *tableName_;
    QTreeWidgetItem *tableOffset_;
    QTreeWidgetItem *tableWidth_;
    QTreeWidgetItem *tableHeight_;
    QTreeWidgetItem *tableRange_;
};

#endif // SIDEBARWIDGET_H
