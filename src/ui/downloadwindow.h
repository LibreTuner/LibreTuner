#ifndef DOWNLOADWINDOW_H
#define DOWNLOADWINDOW_H

#include <QWidget>

#include <memory>

#include "downloadinterface.h"
#include "rommanager.h"

namespace Ui
{
class DownloadWindow;
}

/**
 * Window for downloading firmware from the ECU
 */
class DownloadWindow : public QWidget, public DownloadInterface::Callbacks
{
    Q_OBJECT
public:
    DownloadWindow(QWidget *parent =0);
    ~DownloadWindow();
    
    
    /* Download interface callbacks */
    void downloadError(const QString & error) override;
    void onCompletion(const uint8_t * data, size_t length) override;
    void updateProgress(float progress) override;
    
    
private slots:
    void on_comboMode_activated(int index);
    void on_buttonContinue_clicked();
    void on_buttonBack_clicked();
    void showEvent(QShowEvent * event) override;
    void closeEvent(QCloseEvent * event) override;

    void mainDownloadError(const QString &error);
    void mainOnCompletion();

private:
    
    Ui::DownloadWindow* ui;
    std::shared_ptr<DownloadInterface> downloadInterface_;
    std::string name_;
    DefinitionPtr definition_;

    void start();
};

#endif // DOWNLOADWINDOW_H
