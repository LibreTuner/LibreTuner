#ifndef LIBRETUNER_DEFINITIONSWINDOW_H
#define LIBRETUNER_DEFINITIONSWINDOW_H

#include <QTreeView>
#include <QWidget>

namespace lt
{
class Platforms;
}

class DefinitionsWindow : public QWidget
{
public:
    explicit DefinitionsWindow(QWidget * parent = nullptr);

    void setDefinitions(lt::Platforms * definitions) noexcept;

private:
    lt::Platforms * definitions_{nullptr};
    QTreeView * platformsView_;
};

#endif // LIBRETUNER_DEFINITIONSWINDOW_H
