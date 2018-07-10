#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>

class QPushButton;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *window);

    void paintEvent(QPaintEvent *event) override;

    QLabel *title_;
signals:

public slots:
    void setTitle(const QString &title);
    void setMaximized(bool maximized);

private:
    QWidget *window_;

    QPushButton *restore_;
    QPushButton *maximize_;
};

#endif // TITLEBAR_H
