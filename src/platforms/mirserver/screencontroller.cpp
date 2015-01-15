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


ScreenController::ScreenController(const QSharedPointer<MirServer> &server,
                                   MirServerIntegration *platformIntegration,
                                   QObject *parent)
    : QObject(parent)
    , m_server(server)
    , m_platformIntegration(platformIntegration)
    , m_watchForUpdates(true)
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::ScreenController";

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
            QMetaObject::invokeMethod(this, "updateScreens", Qt::QueuedConnection);
        }
    });

    updateScreens();
}

void ScreenController::onCompositorStarting()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStarting";
    {
        QMutexLocker lock(&m_mutex);
        m_watchForUpdates = false;
    }

    updateScreens();

    // (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
    for (auto qscreen : m_qscreenList) {
        auto screen = static_cast<Screen *>(qscreen->handle());
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window) {
            window->setVisible(true);
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
    for (auto qscreen : m_qscreenList) {
        auto screen = static_cast<Screen *>(qscreen->handle());
        auto window = static_cast<ScreenWindow *>(screen->window());
        if (window) {
            window->setVisible(false);
        }
    }
}

void ScreenController::updateScreens()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::update";
    auto display = m_server->the_display();
    auto displayConfig = display->configuration();

    QMutexLocker lock(&m_mutex);

    QList<QScreen*> oldQScreenList = m_qscreenList;
    m_qscreenList.clear();

    displayConfig->for_each_output(
        [this, &oldQScreenList](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreen(oldQScreenList, output.id);
                if (screen) { // we've already set up this display before, refresh its internals
                    screen->setMirDisplayConfiguration(output);
                    oldQScreenList.removeAll(screen->screen());
                } else {
                    // new display, so create Screen for it
                    screen = new Screen(output);
                    m_platformIntegration->screenAdded(screen); // notify Qt
                    qCDebug(QTMIR_SCREENS) << "Added Screen with id" << output.id.as_value()
                                           << "and geometry" << screen->geometry();
                }
                m_qscreenList.append(screen->screen());
            }
        }
    );

    // Delete any old & unused Screens
    for (auto screen: oldQScreenList) {
        qCDebug(QTMIR_SCREENS) << "Removed Screen with id" << static_cast<Screen *>(screen->handle())->m_outputId.as_value()
                               << "and geometry" << screen->geometry();
        // The screen is automatically removed from Qt's internal list by the QPlatformScreen deconstructor.
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

            for (auto qscreen : m_qscreenList) {
                if (dbGeom == qscreen->geometry()) {
                    static_cast<Screen *>(qscreen->handle())->setMirDisplayBuffer(&buffer);
                }
            }
        }
    );

    for (auto qscreen: m_qscreenList) {
        qCDebug(QTMIR_SCREENS) << "Screen - id:" << static_cast<Screen *>(qscreen->handle())->m_outputId.as_value()
                               << "geometry" << qscreen->geometry();
    }

}

QScreen* ScreenController::getUnusedQScreen()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::getUnusedQScreen";

    QScreen *unusedQScreen = nullptr;
    // have all existing screens got an associated ScreenWindow?
    for (auto qscreen : m_qscreenList) {
        auto screen = static_cast<Screen*>(qscreen->handle());
        if (screen && !screen->window()) {
            unusedQScreen = qscreen;
            break;
        }
    }

    return unusedQScreen;
}

Screen* ScreenController::findScreen(const QList<QScreen*> &list, const mg::DisplayConfigurationOutputId id)
{
    Screen *screen;
    for (QScreen* qscreen : list) {
        screen = static_cast<Screen *>(qscreen->handle());
        if (screen->m_outputId == id) {
            return screen;
        }
    }
    return nullptr;
}
