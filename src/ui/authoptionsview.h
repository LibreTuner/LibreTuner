#ifndef AUTHOPTIONSVIEW_H
#define AUTHOPTIONSVIEW_H

#include <QWidget>

class QLineEdit;

class AuthOptionsView : public QWidget
{
    Q_OBJECT
public:
    explicit AuthOptionsView(QWidget *parent = nullptr);

signals:

public slots:

private:
    QLineEdit *lineKey_;
};

#endif // AUTHOPTIONSVIEW_H
