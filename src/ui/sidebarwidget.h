#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H


#include <QWidget>
#include <QToolButton>
#include <QTreeWidget>

class SidebarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    
    void setTableName(const QString &name);
    void setTableOffset(std::size_t offset);

private slots:
    void on_treeToolButton_clicked(bool checked);

private:
    QToolButton *tableInfoButton_;
    QTreeWidget *tableTreeWidget_;
    
    QTreeWidgetItem *tableName_;
    QTreeWidgetItem *tableOffset_;
};

#endif // SIDEBARWIDGET_H
