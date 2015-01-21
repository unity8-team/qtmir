/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Gerry Boland <gerry.boland@canonical.com>
 */

#include "screencontroller.h"

#include "screenwindow.h"
#include "qtcompositor.h"
#include "logging.h"
#include "mirserver.h"
#include "mirserverintegration.h"
#include "screen.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/display_buffer.h>
#include <mir/main_loop.h>

// Qt
#include <QMutexLocker>
#include <QScreen>
#include <QQuickWindow>

// std
#include <memory>

Q_LOGGING_CATEGORY(QTMIR_SCREENS, "qtmir.screens")

namespace mg = mir::graphics;


ScreenController::ScreenController(QObject *parent)
    : QObject(parent)
    , m_watchForUpdates(true)
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::ScreenController";
}

void ScreenController::init()
{
    // Using Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    auto compositor = static_cast<QtCompositor *>(m_server->the_compositor().get());
    connect(compositor, &QtCompositor::starting,
            this, &ScreenController::onCompositorStarting, Qt::BlockingQueuedConnection);
    connect(compositor, &QtCompositor::stopping,
            this, &ScreenController::onCompositorStopping, Qt::BlockingQueuedConnection);

    auto display = m_server->the_display();
    display->register_configuration_change_handler(*m_server->the_main_loop(), [this]() {
        // display hardware configuration changed, update! - not called when we set new configuration
        QMutexLocker lock(&m_mutex);
        if (m_watchForUpdates) {
            QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
        }
    });

    update();
}

void ScreenController::onCompositorStarting()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStarting";

    {
        QMutexLocker lock(&m_mutex);
        m_watchForUpdates = false;
    }

    update();

    // (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
    for (auto screen : m_screenList) {
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window()) { qDebug() << "SHOW" << window;
            //window->setVisible(true);
            window->window()->show();
        }
    }
}

void ScreenController::onCompositorStopping()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStopping";

    {
        QMutexLocker lock(&m_mutex);
        m_watchForUpdates = true;
    }

    // Stop Qt's render threads by setting all its windows it obscured. Must
    // block until all windows have their GL contexts released.
    for (auto screen : m_screenList) {
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window()) { qDebug() << "HIDE" << window;
            //window->setVisible(false);
            window->window()->hide();
        }
    }

    update();
}

void ScreenController::update()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::update";
    auto display = m_server->the_display();
    auto displayConfig = display->configuration();

    QMutexLocker lock(&m_mutex);

    Screen *newScreen = nullptr;
    QList<Screen*> oldScreenList = m_screenList;
    m_screenList.clear();

    displayConfig->for_each_output(
        [this, &oldScreenList, &newScreen](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreenWithId(oldScreenList, output.id);
                if (screen) { // we've already set up this display before, refresh its internals
                    screen->setMirDisplayConfiguration(output);
                    oldScreenList.removeAll(screen);
                } else {
                    // new display, so create Screen for it
                    screen = new Screen(output);
                    newScreen = screen;
                    qCDebug(QTMIR_SCREENS) << "Added Screen with id" << output.id.as_value()
                                           << "and geometry" << screen->geometry();
                }
                m_screenList.append(screen);
            }
        }
    );

    // Delete any old & unused Screens
    for (auto screen: oldScreenList) {
        qCDebug(QTMIR_SCREENS) << "Removed Screen with id" << screen->m_outputId.as_value()
                               << "and geometry" << screen->geometry();
        // The screen is automatically removed from Qt's internal list by the QPlatformScreen deconstructor.
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window && window->window() && window->isExposed()) { qDebug() << "HIDE!" << window;
            //window->setVisible(false);
            window->window()->hide();
        }
        delete screen;
    }

    // Match up the new Mir DisplayBuffers with each Screen
    display->for_each_display_buffer(
        [&](mg::DisplayBuffer& buffer) {
            // only way to match Screen to a DisplayBuffer is by matching the geometry
            QRect dbGeom(buffer.view_area().top_left.x.as_int(),
                         buffer.view_area().top_left.y.as_int(),
                         buffer.view_area().size.width.as_int(),
                         buffer.view_area().size.height.as_int());

            for (auto screen : m_screenList) {
                if (dbGeom == screen->geometry()) {
                    screen->setMirDisplayBuffer(&buffer);
                }
            }
        }
    );

    qCDebug(QTMIR_SCREENS) << "=======================================";
    for (auto screen: m_screenList) {
        qCDebug(QTMIR_SCREENS) << "Screen - id:" << screen->m_outputId.as_value()
                               << "geometry:" << screen->geometry()
                               << "buffer:" << screen->mirDisplayBuffer()
                               << "window:" << screen->window();
    }
    qCDebug(QTMIR_SCREENS) << "=======================================";

    if (newScreen) {
        Q_EMIT screenAdded(newScreen);
    }
}

Screen* ScreenController::getUnusedScreen()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::getUnusedScreen";

    // have all existing screens got an associated ScreenWindow?
    for (auto screen : m_screenList) {
        if (!screen->window()) {
            return screen;
        }
    }

    return nullptr;
}

Screen* ScreenController::findScreenWithId(const QList<Screen *> &list, const mg::DisplayConfigurationOutputId id)
{
    for (Screen *screen : list) {
        if (screen->m_outputId == id) {
            return screen;
        }
    }
    return nullptr;
}

QWindow* ScreenController::getWindowForPoint(const QPoint &point) //HORRIBLE!!!
{
    QMutexLocker lock(&m_mutex);
    for (Screen *screen : m_screenList) {
        if (screen->window() && screen->geometry().contains(point)) {
            return screen->window()->window();
        }
    }
    return nullptr;
}
