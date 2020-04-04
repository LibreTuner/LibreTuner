#ifndef FLASHDIALOG_H
#define FLASHDIALOG_H

#include <QWizard>
#include <link/datalink.h>

namespace Ui
{
class FlashDialog;
}

class FlashDialog : public QWizard
{
    Q_OBJECT

public:
    explicit FlashDialog(QWidget * parent = nullptr);
    ~FlashDialog();

    // Returns DataLink pointer or nullptr if none is selected.
    // The lifetime of the DataLink depends on this dialog.
    lt::DataLink * dataLink() const;

private:
    Ui::FlashDialog * ui;

    std::vector<lt::DataLinkPtr> links_;
};

#endif // FLASHDIALOG_H
