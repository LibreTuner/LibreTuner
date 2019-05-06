#ifndef LIBRETUNER_DEFINITIONSWINDOW_H
#define LIBRETUNER_DEFINITIONSWINDOW_H

#include <QWidget>
#include <QTreeView>

class Definitions;

class DefinitionsWindow : public QWidget {
public:
    explicit DefinitionsWindow(QWidget *parent = nullptr);

    void setDefinitions(Definitions *definitions) noexcept;

private:
    Definitions *definitions_{nullptr};
    QTreeView *platformsView_;
};


#endif //LIBRETUNER_DEFINITIONSWINDOW_H
