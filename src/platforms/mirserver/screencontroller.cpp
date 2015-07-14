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
#include <QScreen>
#include <QQuickWindow>
#include <qpa/qwindowsysteminterface.h>

// std
#include <memory>

Q_LOGGING_CATEGORY(QTMIR_SCREENS, "qtmir.screens")

namespace mg = mir::graphics;

/*
 * ScreenController monitors the Mir display configuration and compositor status, and updates
 * the relevant QScreen and QWindow states accordingly.
 *
 * Primary purposes are:
 * 1. to update QScreen state on Mir display configuration changes
 * 2. to stop the Qt renderer by hiding its QWindow when Mir wants to stop all compositing,
 *    and resume Qt's renderer by showing its QWindow when Mir wants to resume compositing.
 *
 *
 * Threading Note:
 * This object has affinity to the main Qt GUI thread. However the init() & terminate() methods
 * are called on the MirServerThread thread, as we need to determine the screen state *after*
 * Mir has initialized, and tear down before Mir terminates. Also note the MirServerThread
 * does not have an QEventLoop.
 *
 * All other methods must be called on the Qt GUI thread.
 */

ScreenController::ScreenController(QObject *parent)
    : QObject(parent)
    , m_server(nullptr)
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::ScreenController";
}

// init only after MirServer has initialized - runs on MirServerThread!!!
void ScreenController::init(MirServer *server)
{
    m_server = server;

    // Use a Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    // on compositor shutdown. Compositor startup can be lazy.
    // Queued connections work because the thread affinity of this class is with the Qt GUI thread.
    auto compositor = static_cast<QtCompositor *>(m_server->the_compositor().get());
    connect(compositor, &QtCompositor::starting,
            this, &ScreenController::onCompositorStarting);
    connect(compositor, &QtCompositor::stopping,
            this, &ScreenController::onCompositorStopping, Qt::BlockingQueuedConnection);

    auto display = m_server->the_display();
    display->register_configuration_change_handler(*m_server->the_main_loop(), [this]() {
        // display hardware configuration changed, update! - not called when we set new configuration
        QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
    });
}

// terminate before shutting down the Mir server, or else liable to deadlock with the blocking connection above
// Runs on MirServerThread!!!
void ScreenController::terminate()
{
    auto compositor = static_cast<QtCompositor *>(m_server->the_compositor().get());
    disconnect(compositor, 0, 0, 0);
    m_server = nullptr;
}

void ScreenController::onCompositorStarting()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::onCompositorStarting";

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
    if (!m_server)
        return;
    auto display = m_server->the_display();
    auto displayConfig = display->configuration();

    // Mir only tells us something changed, it is up to us to figure out what.
    QList<Screen*> newScreenList;
    QList<Screen*> oldScreenList = m_screenList;
    m_screenList.clear();

    displayConfig->for_each_output(
        [this, &oldScreenList, &newScreenList](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreenWithId(oldScreenList, output.id);
                if (screen) { // we've already set up this display before, refresh its internals
                    screen->setMirDisplayConfiguration(output);
                    oldScreenList.removeAll(screen);
                } else {
                    // new display, so create Screen for it
                    screen = new Screen(output);
                    newScreenList.append(screen);
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
    display->for_each_display_sync_group([&](mg::DisplaySyncGroup &group) {
        group.for_each_display_buffer([&](mg::DisplayBuffer &buffer) {
            // only way to match Screen to a DisplayBuffer is by matching the geometry
            QRect dbGeom(buffer.view_area().top_left.x.as_int(),
                         buffer.view_area().top_left.y.as_int(),
                         buffer.view_area().size.width.as_int(),
                         buffer.view_area().size.height.as_int());

            for (auto screen : m_screenList) {
                if (dbGeom == screen->geometry()) {
                    screen->setMirDisplayBuffer(&buffer, &group);
                }
            }
        });
    });

    qCDebug(QTMIR_SCREENS) << "=======================================";
    for (auto screen: m_screenList) {
        qCDebug(QTMIR_SCREENS) << "Screen - id:" << screen->m_outputId.as_value()
                               << "geometry:" << screen->geometry()
                               << "window:" << screen->window();
    }
    qCDebug(QTMIR_SCREENS) << "=======================================";

    for (auto screen : newScreenList) {
        Q_EMIT screenAdded(screen);
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
    for (Screen *screen : m_screenList) {
        if (screen->window() && screen->geometry().contains(point)) {
            return screen->window()->window();
        }
    }
    return nullptr;
}
