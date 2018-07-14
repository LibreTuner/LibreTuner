#ifndef STYLEDWINDOW_H
#define STYLEDWINDOW_H

#include <QWidget>
#include <QDialog>
#include <type_traits>

class TitleBar;

/* A window with custom borders and a title bar on supported platforms */

template<class T>
class StyledWidget : public T
{
public:
    explicit StyledWidget(QWidget *parent = nullptr);

    QLayout *mainLayout() {
        return layout_;
    }

    void setResizable(bool resizable);

#ifdef _WIN32
  bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
  void changeEvent(QEvent *e) override;
#endif
signals:

public slots:

protected:
#ifdef _WIN32
  TitleBar *titleBar_;
#endif
  QLayout *layout_;

private:
  bool resizable_ = true;
};

class StyledWindow : public StyledWidget<QWidget>
{
    Q_OBJECT
public:
    explicit StyledWindow(QWidget *parent = nullptr);
};

class StyledDialog : public StyledWidget<QDialog>
{
    Q_OBJECT
public:
    explicit StyledDialog(QWidget *parent = nullptr);
};

#endif // STYLEDWINDOW_H
