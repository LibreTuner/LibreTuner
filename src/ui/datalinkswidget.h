#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include <QAbstractItemModel>
#include <QWidget>

class QPushButton;
class QTreeView;
class DataLinkSettings;

namespace lt {
class DataLink;
}

class DatalinksWidget : public QWidget {
public:
    explicit DatalinksWidget(QWidget *parent = nullptr);

private:
    void linkChanged(lt::DataLink *link);
    lt::DataLink *currentLink() const;

    void setButtonsEnabled(bool enabled);

    QPushButton *buttonUpdate_;
    QPushButton *buttonReset_;
    QTreeView *linksView_;
    DataLinkSettings *settings_;
};

#endif // LIBRETUNER_INTERFACESDIALOG_H
