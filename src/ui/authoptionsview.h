#ifndef AUTHOPTIONSVIEW_H
#define AUTHOPTIONSVIEW_H

#include <QWidget>
#include "udsauthenticator.h"

class QLineEdit;
class QSpinBox;

class AuthOptionsView : public QWidget
{
    Q_OBJECT
public:
    explicit AuthOptionsView(QWidget *parent = nullptr);
    
    void setDefaultOptions(const auth::Options &options);

signals:

public slots:
    void resetOptions();

private:
    QLineEdit *lineKey_;
    QSpinBox *spinSessionId_;
    
    auth::Options defaultOptions_;
};

#endif // AUTHOPTIONSVIEW_H
