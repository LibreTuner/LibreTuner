#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "canviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionCAN_Log_triggered();
    void closeEvent(QCloseEvent * event) override;

private:
    Ui::MainWindow *ui;
    CanViewer canViewer_;
};

#endif // MAINWINDOW_H
