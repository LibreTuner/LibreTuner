#ifndef LIBRETUNER_SCALARVIEW_H
#define LIBRETUNER_SCALARVIEW_H

#include <QWidget>

namespace lt
{
template<typename PresentedType>
class BasicTable;
using Table = BasicTable<double>;
}

class QLineEdit;
class QSlider;
class QLabel;
class QPushButton;

// A scalar is just a 1x1 table
class ScalarView : public QWidget {
public:
    explicit ScalarView(QWidget * parent = nullptr);

    void setTable(lt::Table * table);

protected:
    void closeEvent(QCloseEvent * event) override;

public slots:
    void setValue(double value);
    void save();

private:
    lt::Table * table_{nullptr};
    bool dirty_{false};

    void setDirty(bool dirty);

    QLineEdit * lineValue_;
    QSlider * slider_;
    QLabel * description_;
    QPushButton * buttonResetBase_;
    QPushButton * buttonReset_;
    QPushButton * buttonSave_;
};



#endif //LIBRETUNER_SCALARVIEW_H
