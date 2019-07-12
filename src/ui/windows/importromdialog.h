#ifndef LIBRETUNER_IMPORTROMDIALOG_H
#define LIBRETUNER_IMPORTROMDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;

namespace lt
{
    class Project;
    using ProjectPtr = std::shared_ptr<Project>;
}

class ImportRomDialog : public QDialog
{
public:
    explicit ImportRomDialog(lt::ProjectPtr project = lt::ProjectPtr(), QWidget * parent = nullptr);

public slots:
    void platformChanged(int index);

private:
    QLineEdit * lineName_;
    QLineEdit * linePath_;
    QComboBox * comboPlatform_;

    lt::ProjectPtr  project_;
};

#endif // LIBRETUNER_IMPORTROMDIALOG_H
