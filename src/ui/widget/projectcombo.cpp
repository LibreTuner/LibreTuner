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

void ProjectCombo::setProject(const lt::ProjectPtr & project)
{
    if (!project)
        return;

    for (int i = 0; i < model()->rowCount(); ++i) {
        QVariant v = model()->data(model()->index(i, 0));
        if (v.canConvert<lt::ProjectPtr>() && v.value<lt::ProjectPtr>() == project)
            setCurrentIndex(i);
    }
}
