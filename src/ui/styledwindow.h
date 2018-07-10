#ifndef STYLEDWINDOW_H
#define STYLEDWINDOW_H

#include <QWidget>

class TitleBar;

/* A window with custom borders and a title bar on supported platforms */
class StyledWindow : public QWidget
{
    Q_OBJECT
public:
    explicit StyledWindow(QWidget *parent = nullptr);

    QLayout *mainLayout() {
        return layout_;
    }

#ifdef _WIN32
  bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
  void changeEvent(QEvent *e) override;
#endif
signals:

public slots:

private:
#ifdef _WIN32
  TitleBar *titleBar_;
#endif
  QLayout *layout_;
};

#endif // STYLEDWINDOW_H
