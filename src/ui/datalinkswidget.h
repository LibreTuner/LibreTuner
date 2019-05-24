#ifndef LIBRETUNER_INTERFACESDIALOG_H
#define LIBRETUNER_INTERFACESDIALOG_H

#include <QAbstractItemModel>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QTreeView;
class QSpinBox;
class QComboBox;
class CustomCombo;
class QCheckBox;

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

    QLineEdit *lineName_;
    CustomCombo *comboPort_;
    QSpinBox *spinBaudrate_;
    QCheckBox *checkBaudrate_;

    QPushButton *buttonUpdate_;
    QPushButton *buttonReset_;
    QTreeView *linksView_;
};

#endif // LIBRETUNER_INTERFACESDIALOG_H
