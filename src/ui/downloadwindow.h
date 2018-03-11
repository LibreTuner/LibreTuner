#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QWidget>

namespace Ui
{
class DownloadWindow;
}

/**
 * Window for downloading firmware from the ECU
 */
class DownloadWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadWindow(QWidget *parent =0);
    ~DownloadWindow();
    
private slots:
    void on_comboMode_activated(const QString &text);

private:
    Ui::DownloadWindow* ui;
};

#endif // DOWNLOADWINDOW_H
