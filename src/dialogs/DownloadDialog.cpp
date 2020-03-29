#include "DownloadDialog.h"
#include "ui_DownloadDialog.h"
#include <OverboostApp.h>
#include <link/passthru.h>

Q_DECLARE_METATYPE(lt::DataLink *)

DownloadDialog::DownloadDialog(QWidget * parent)
    : QWizard(parent), ui(new Ui::DownloadDialog), platformsModel_(&OB()->platforms())
{
    ui->setupUi(this);

    for (auto & passthru_link : lt::detect_passthru_links())
    {
        lt::DataLinkPtr & link = links_.emplace_back(std::move(passthru_link));
        ui->comboInterfaces->addItem(QString::fromStdString(link->name()), QVariant::fromValue(link.get()));
    }

    ui->comboPlatform->setModel(&platformsModel_);
}

DownloadDialog::~DownloadDialog() { delete ui; }

lt::DataLink * DownloadDialog::dataLink() const
{
    return ui->comboInterfaces->currentData(Qt::UserRole).value<lt::DataLink *>();
}

const lt::Platform * DownloadDialog::platform() const
{
    return ui->comboPlatform->currentData(Qt::UserRole).value<const lt::Platform *>();
}
