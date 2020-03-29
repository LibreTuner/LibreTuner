#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QWizard>
#include <link/datalink.h>
#include <models/PlatformsModel.h>

namespace Ui
{
class DownloadDialog;
}

class DownloadDialog : public QWizard
{
    Q_OBJECT

public:
    explicit DownloadDialog(QWidget * parent = nullptr);
    ~DownloadDialog();

    // Returns DataLink pointer or nullptr if none is selected.
    // The lifetime of the DataLink depends on this dialog.
    lt::DataLink * dataLink() const;

    // Returns the selected platform
    const lt::Platform * platform() const;

private:
    Ui::DownloadDialog * ui;

    PlatformsModel platformsModel_;

    std::vector<lt::DataLinkPtr> links_;
};

#endif // DOWNLOADDIALOG_H
