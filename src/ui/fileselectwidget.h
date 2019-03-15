#ifndef FILESELECTWIDGET_H
#define FILESELECTWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;

class FileSelectWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        MODE_OPEN,
        MODE_SAVE,
    };

    FileSelectWidget(const QString &caption = QString(), const QString &filter = QString(), Mode mode = MODE_OPEN, QWidget *parent = nullptr);

    inline void setCaption(const QString &caption) noexcept { caption_ = caption; }
    inline void setFilter(const QString &filter) noexcept { filter_ = filter; }
    inline void setMode(Mode mode) noexcept { mode_ = mode; }

    void setPath(const QString &path) noexcept;
    QString path() const noexcept;

signals:
    void pathChanged(const QString &path);

public slots:

private slots:
    void browse();

private:
    QLineEdit *linePath_;
    QPushButton *buttonBrowse_;

    QString caption_;
    QString filter_;
    Mode mode_{MODE_OPEN};
};

#endif // FILESELECTWIDGET_H
