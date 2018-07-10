#include "titlebar.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QApplication>

#ifdef _WIN32
#include <windows.h>
#endif

TitleBar::TitleBar(QWidget *window) : QWidget(window), window_(window)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(30);

    QLabel *icon = new QLabel();
    layout->addWidget(icon);
    icon->setPixmap(QPixmap(":/icons/LibreTuner.png").scaled(QSize(20, 20), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setFixedSize(30, 30);

    title_ = new QLabel();
    layout->addWidget(title_);
    title_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->setContentsMargins(5, 0, 0, 0);

    QPushButton *minimize = new QPushButton();
    layout->addWidget(minimize);
    minimize->setIcon(QIcon(":/icons/minimize.svg"));
    minimize->setIconSize(QSize(12, 12));
    minimize->setFixedSize(50, 30);
    minimize->setFlat(true);
    connect(minimize, &QPushButton::clicked, [this]() {
#ifdef _WIN32
        ShowWindow(reinterpret_cast<HWND>(window_->winId()), SW_MINIMIZE);
#else
        window_->showMinimized();
#endif
    });

    maximize_ = new QPushButton();
    layout->addWidget(maximize_);
    maximize_->setIcon(QIcon(":/icons/maximize.svg"));
    maximize_->setIconSize(QSize(12, 12));
    maximize_->setFixedSize(50, 30);
    maximize_->setFlat(true);
    connect(maximize_, &QPushButton::clicked, [this]() {
        window_->setWindowState(Qt::WindowMaximized);
    });

    restore_ = new QPushButton();
    layout->addWidget(restore_);
    restore_->setVisible(false);
    restore_->setIcon(QIcon(":/icons/restore.svg"));
    restore_->setIconSize(QSize(12, 12));
    restore_->setFixedSize(50, 30);
    restore_->setFlat(true);
    connect(restore_, &QPushButton::clicked, [this]() {
       window_->setWindowState(Qt::WindowNoState);
    });

    QPushButton *close = new QPushButton();
    layout->addWidget(close);
    close->setIcon(QIcon(":/icons/close.svg"));
    close->setIconSize(QSize(12, 12));
    close->setFixedSize(50, 30);
    close->setFlat(true);
    connect(close, &QPushButton::clicked, [this]() {
        window_->close();
    });

    setLayout(layout);
    setAutoFillBackground(true);

    //setMaximized(true);
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}

void TitleBar::setTitle(const QString &title)
{
    title_->setText(title);
}

void TitleBar::setMaximized(bool maximized)
{
    restore_->setVisible(maximized);
    maximize_->setVisible(!maximized);

    if (maximized) {
        maximize_->setAttribute(Qt::WA_UnderMouse, false);
    } else {
        restore_->setAttribute(Qt::WA_UnderMouse, false);
    }
}

