/*
 * Copyright (C) 2013,2014 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Gerry Boland <gerry.boland@canonical.com>
 *          Daniel d'Andrada <daniel.dandrada@canonical.com>
 */

#include "screenwindow.h"
#include "screen.h"

#include <mir/geometry/size.h>
#include <mir/graphics/display_buffer.h>

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformscreen.h>

#include <QDebug>

static WId newWId()
{
    static WId id = 0;

    if (id == std::numeric_limits<WId>::max())
        qWarning("MirServer QPA: Out of window IDs");

    return ++id;
}

ScreenWindow::ScreenWindow(QWindow *window)
    : QObject(nullptr), QPlatformWindow(window)
    , m_isExposed(false)
    , m_winId(newWId())
{
    qDebug() << "ScreenWindow::ScreenWindow" << this;
    qWarning("Window %p: %p 0x%x\n", this, window, uint(m_winId));

    // Register with the Screen it is associated with
    auto windowscreen = static_cast<Screen *>(screen());
    Q_ASSERT(windowscreen);
    windowscreen->setWindow(this);

    QRect screenGeometry(screen()->availableGeometry());
    if (window->geometry() != screenGeometry) {
        setGeometry(screenGeometry);
    }
    window->setSurfaceType(QSurface::OpenGLSurface);

    // The compositor window is always active. I.e., it's always focused so that
    // it always processes key events, etc
    requestActivateWindow();
}

ScreenWindow::~ScreenWindow()
{
    qDebug() << "ScreenWindow::~ScreenWindow" << this;
}

//bool ScreenWindow::isExposed() const
//{
//    return m_isExposed;
//}

bool ScreenWindow::event(QEvent *event)
{
    // Intercept Hide event and convert to Expose event, as Hide causes Qt to release GL
    // resources, which we don't want. Must intercept Show to un-do hide.
    if (event->type() == QEvent::Hide) {
        qDebug() << "ScreenWindow::event got QEvent::Hide";
        m_isExposed = false;
        //QWindowSystemInterface::handleExposeEvent(window(), QRect());
        //QWindowSystemInterface::flushWindowSystemEvents();
        //return true;
    } else if (event->type() == QEvent::Show) {
        qDebug() << "ScreenWindow::event got QEvent::Show";
        m_isExposed = true;
        //QRect rect(QPoint(), geometry().size());
        //QWindowSystemInterface::handleExposeEvent(window(), rect);
        //QWindowSystemInterface::flushWindowSystemEvents();
        //return true;
    }
    return QObject::event(event);
}

void ScreenWindow::swapBuffers()
{
    static_cast<Screen *>(screen())->swapBuffers();
}

void ScreenWindow::makeCurrent()
{
    static_cast<Screen *>(screen())->makeCurrent();
}

void ScreenWindow::doneCurrent()
{
    qDebug() << "ScreenWindow::doneCurrent";
    static_cast<Screen *>(screen())->doneCurrent();
}

