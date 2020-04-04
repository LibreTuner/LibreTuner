#include "FlashDialog.h"
#include "ui_FlashDialog.h"
#include <link/passthru.h>

Q_DECLARE_METATYPE(lt::DataLink *)

FlashDialog::FlashDialog(QWidget * parent) : QWizard(parent), ui(new Ui::FlashDialog)
{
    ui->setupUi(this);

    for (auto & passthru_link : lt::detect_passthru_links())
    {
        lt::DataLinkPtr & link = links_.emplace_back(std::move(passthru_link));
        ui->comboInterfaces->addItem(QString::fromStdString(link->name()), QVariant::fromValue(link.get()));
    }
}

FlashDialog::~FlashDialog() { delete ui; }

lt::DataLink * FlashDialog::dataLink() const
{
    return ui->comboInterfaces->currentData(Qt::UserRole).value<lt::DataLink *>();
}
