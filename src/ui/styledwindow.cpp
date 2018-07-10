#include "styledwindow.h"
#include "titlebar.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QEvent>
#include <QWindowStateChangeEvent>

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>


// Parts of this were taken from https://github.com/Moussa-Ball/BorderlessWindowQt-Modern-Gui :)
namespace {

bool maximized(HWND hwnd) {
    WINDOWPLACEMENT placement;
    if (!GetWindowPlacement(hwnd, &placement)) {
        return false;
    }

    return placement.showCmd == SW_MAXIMIZE;
}

void adjust_maximized_client_rect(HWND window, RECT& rect) {
    if (!maximized(window)) {
        return;
    }

    auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
    if (!monitor) {
        return;
    }

    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(monitor_info);
    if (!GetMonitorInfoW(monitor, &monitor_info)) {
        return;
    }

    rect = monitor_info.rcWork;
}

bool composition_enabled() {
    //BOOL composition_enabled = FALSE;
    //bool success = DwmIsCompositionEnabled(&composition_enabled) == S_OK;
    //return composition_enabled && success;
    return true;
}

}
#endif

StyledWindow::StyledWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::Window);
    setObjectName("mainWindow");
    layout_ = new QVBoxLayout;
    layout_->setSpacing(0);

#ifdef _WIN32
    layout_->setMargin(1);
    titleBar_ = new TitleBar(this);
    titleBar_->setTitle("Test");

    layout_->addWidget(titleBar_);

    HWND hwnd = reinterpret_cast<HWND>(winId());
    SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(hwnd, SW_SHOW);
#else
    layout_->setMargin(0);
    setWindowFlags(Qt::FramelessWindowHint);
#endif

    setLayout(layout_);
}

#include <iostream>

#ifdef _WIN32
bool StyledWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    MSG *msg = static_cast<MSG*>(message);

    switch (msg->message) {
    case WM_NCCALCSIZE: {
        if (msg->wParam == TRUE) {
            auto &params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            adjust_maximized_client_rect(msg->hwnd, params.rgrc[0]);
        }
        *result = 0;
        return true;
    }
    case WM_NCHITTEST: {
        static const QMargins margins(5, 5, 5, 5);
        RECT winrect;
        GetWindowRect(reinterpret_cast<HWND>(winId()), &winrect);

        QPoint topLeft(winrect.left, winrect.top);
        QPoint bottomRight(winrect.right, winrect.bottom);
        QRect rect = QRect(topLeft, bottomRight);

        QRect leftBorder(rect.topLeft(), rect.bottomLeft() + QPoint(margins.left(), 0));
        QRect topBorder(rect.topLeft(), rect.topRight() + QPoint(0, margins.top()));
        QRect rightBorder(rect.topRight() - QPoint(margins.right(), 0), rect.bottomRight());
        QRect bottomBorder(rect.bottomLeft() - QPoint(0, margins.bottom()), rect.bottomRight());

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();

        *result = 0;
        if (resizeWidth) {
            if (resizeHeight) {
                //bottom left corner
                if (leftBorder.contains(x, y) && bottomBorder.contains(x, y))
                {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                //bottom right corner
                if (bottomBorder.contains(x, y) && rightBorder.contains(x, y))
                {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                //top left corner
                if (leftBorder.contains(x, y) && topBorder.contains(x, y))
                {
                    *result = HTTOPLEFT;
                    return true;
                }
                //top right corner
                if (rightBorder.contains(x, y) && topBorder.contains(x, y))
                {
                    *result = HTTOPRIGHT;
                    return true;
                }
            }
            if (leftBorder.contains(x, y)) {
                // Left border
                *result = HTLEFT;
                return true;
            }
            if (rightBorder.contains(x, y)) {
                // Right border
                *result = HTRIGHT;
                return true;
            }
        }
        if (resizeHeight) {
            if (bottomBorder.contains(x, y)) {
                // Bottom border
                *result = HTBOTTOM;
                return true;
            }
            if (topBorder.contains(x, y)) {
                // Top border
                *result = HTTOP;
                return true;
            }
        }

        if (QApplication::widgetAt(QCursor::pos()) == titleBar_->title_) {
            *result = HTCAPTION;
            return true;
        }
        // Otherwise, let QWidget handle it.
        break;
    }
    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);

        if (maximized(msg->hwnd)) {
            RECT window_rect;

            if (!GetWindowRect(msg->hwnd, &window_rect)) {
                return false;
            }

            HMONITOR monitor = MonitorFromRect(&window_rect, MONITOR_DEFAULTTONULL);
            if (!monitor) {
                return false;
            }

            MONITORINFO monitor_info = {0};
            monitor_info.cbSize = sizeof(monitor_info);
            GetMonitorInfo(monitor, &monitor_info);

            RECT work_area = monitor_info.rcWork;
            RECT monitor_rect = monitor_info.rcMonitor;

            mmi->ptMaxPosition.x = abs(work_area.left - monitor_rect.left);
            mmi->ptMaxPosition.y = abs(work_area.top - monitor_rect.top);

            mmi->ptMaxSize.x = abs(work_area.right - work_area.left);
            mmi->ptMaxSize.y = abs(work_area.bottom - work_area.top);
            mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
            mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;

            *result = 1;
            return true;
        }

        break;
    }

    case WM_SIZE: {
        RECT winrect;
        GetClientRect(msg->hwnd, &winrect);

        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(msg->hwnd, &wp);
        if (wp.showCmd == SW_MAXIMIZE) {
            SetWindowPos(reinterpret_cast<HWND>(winId()), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
        }
        break;
    }
    }
    return QWidget::nativeEvent(eventType, message, result);
}

void StyledWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent *ev = static_cast<QWindowStateChangeEvent*>(e);
        titleBar_->setMaximized(!(ev->oldState() & Qt::WindowMaximized) && (windowState() & Qt::WindowMaximized));
    }

    QWidget::changeEvent(e);
}
#endif
