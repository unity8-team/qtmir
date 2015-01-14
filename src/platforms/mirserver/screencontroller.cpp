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

#include "displaywindow.h"
#include "qtcompositor.h"
#include "logging.h"
#include "mirserver.h"
#include "mirserverintegration.h"
#include "screen.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/main_loop.h>

// Qt
#include <QGuiApplication>
#include <QMutexLocker>
#include <QScreen>
#include <QQuickWindow>
#include <QtQuick/private/qsgrenderloop_p.h>

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
    {
        QMutexLocker lock(&m_mutex);
        m_watchForUpdates = false;
    }

    updateScreens();

    // (Re)Start Qt's render thread by setting all windows with a corresponding screen to exposed.
    auto renderLoop = QSGRenderLoop::instance();
    for (auto window : renderLoop->windows()) {
        for (auto screen : QGuiApplication::screens()) {
            if (window->screen() == screen) {
                renderLoop->show(window);
            }
        }
    }
}

void ScreenController::onCompositorStopping()
{
    {
        QMutexLocker lock(&m_mutex);
        m_watchForUpdates = true;
    }

    // Stop Qt's render threads by setting all its windows it obscured. Must
    // block until all windows have their GL contexts released.
    auto renderLoop = QSGRenderLoop::instance();
    for (auto window : renderLoop->windows()) {
        renderLoop->hide(window);
    }
}

void ScreenController::updateScreens()
{
    qCDebug(QTMIR_SCREENS) << "ScreenController::update";
    std::shared_ptr<mg::DisplayConfiguration> displayConfig = m_server->the_display()->configuration();

    QList<QScreen*> oldScreenList = QGuiApplication::screens();

    displayConfig->for_each_output(
        [this, &oldScreenList](const mg::DisplayConfigurationOutput &output) {
            if (output.used && output.connected) {
                Screen *screen = findScreen(oldScreenList, output.id);
                if (screen) { // we've already set up this display before, refresh its internals
                    screen->setMirDisplayConfiguration(output);
                    oldScreenList.removeAll(screen->screen());
                } else {
                    // new display, so create Screen for it
                    screen = new Screen(output);
                    m_platformIntegration->screenAdded(screen); // notify Qt
                    qCDebug(QTMIR_SCREENS) << "Added Screen with id" << output.id.as_value()
                                           << "and geometry" << screen->geometry();
                }
            }
        }
    );

    // Delete any old & unused Screens
    for (auto screen: oldScreenList) {
        qCDebug(QTMIR_SCREENS) << "Removed Screen with id" << static_cast<Screen *>(screen->handle())->m_outputId.as_value()
                               << "and geometry" << screen->geometry();
        // The screen is automatically removed from Qt's internal list by the QPlatformScreen deconstructor.
        delete screen;
    }
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
