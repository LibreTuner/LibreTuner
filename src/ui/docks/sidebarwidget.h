#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H


#include <QWidget>

namespace lt {
class ModelTable;
}

class QPlainTextEdit;
class QLineEdit;
class QToolButton;

class SidebarWidget : public QWidget {
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    
public slots:
    void fillTableInfo(const lt::ModelTable *table);

private slots:
    void on_treeToolButton_clicked(bool checked);

private:
    QToolButton *tableInfoButton_;
    QWidget *tableInfo_;
    
    QPlainTextEdit *tableDescription_;
    
    QLineEdit *tableName_;
    QLineEdit *tableOffset_;
    QLineEdit *tableWidth_;
    QLineEdit *tableHeight_;
    QLineEdit *tableRange_;
    QLineEdit *tableUnit_;
    QLineEdit *tableDataType_;
};

#endif // SIDEBARWIDGET_H
