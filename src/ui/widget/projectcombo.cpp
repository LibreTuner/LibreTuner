#include "projectcombo.h"

#include <QStyledItemDelegate>
#include <libretuner.h>

ProjectCombo::ProjectCombo(QWidget * parent) : QComboBox(parent)
{
    setItemDelegate(new QStyledItemDelegate());
    setModel(&LT()->projects());
}

lt::ProjectPtr ProjectCombo::selectedProject()
{
    QVariant var = currentData(Qt::UserRole);
    if (!var.canConvert<lt::ProjectPtr>())
        return lt::ProjectPtr();

    return var.value<lt::ProjectPtr>();
}
