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

    void setMinimizable(bool minimizable);
    void setMaximizable(bool maximizable);
signals:

public slots:
    void setTitle(const QString &title);
    void setMaximized(bool maximized);

private:
    QWidget *window_;

    QPushButton *restore_;
    QPushButton *maximize_;
    QPushButton *close_;
    QPushButton *minimize_;

    bool maximizable_ = true;
};

#endif // TITLEBAR_H
