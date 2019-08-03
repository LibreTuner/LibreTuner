#ifndef LIBRETUNER_PROJECTCOMBO_H
#define LIBRETUNER_PROJECTCOMBO_H

#include <QComboBox>
#include <memory>

namespace lt
{
    class Project;
    using ProjectPtr = std::shared_ptr<Project>;
}

class ProjectCombo : public QComboBox
{
    Q_OBJECT
public:
    explicit ProjectCombo(QWidget * parent = nullptr);

    lt::ProjectPtr selectedProject();
};

#endif // LIBRETUNER_PROJECTCOMBO_H
