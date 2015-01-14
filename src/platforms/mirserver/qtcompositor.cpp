/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "qtcompositor.h"
#include "displaywindow.h"
#include "mirserverintegration.h"
#include "logging.h"
#include "screen.h"

#include <mir/graphics/display.h>
#include <mir/graphics/display_configuration.h>

#include <QGuiApplication>
#include <QQuickWindow>
#include <QScreen>
#include <QtQuick/private/qsgrenderloop_p.h>

#include <QDebug>

Q_LOGGING_CATEGORY(QTMIR_MIR_DISPLAYS, "qtmir.displays")

namespace mg = mir::graphics;

QtCompositor::QtCompositor(const std::shared_ptr<mir::graphics::Display> &display,
                           MirServerIntegration *platformIntegration)
    : m_running(false)
    , m_display(display)
    , m_platformIntegration(platformIntegration)
{
    // Using Blocking Queued Connection to enforce synchronization of Qt GUI thread with Mir thread(s)
    connect(this, &QtCompositor::starting, this, &QtCompositor::onStarting, Qt::BlockingQueuedConnection);
    connect(this, &QtCompositor::stopping, this, &QtCompositor::onStopping, Qt::BlockingQueuedConnection);
}

QtCompositor::~QtCompositor()
{
    stop();
}

void QtCompositor::start() // called by Mir (in some Mir thread)
{
    qCDebug(QTMIR_MIR_DISPLAYS) << "QtCompositor::start";
    QMutexLocker lock(&m_runningMutex);
    if (m_running) {
        return;
    }
    m_running = true;

    Q_EMIT onStarting(); // blocks
}

void QtCompositor::onStarting() // in GUI thread
{
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

void QtCompositor::stop() // called by Mir (in some Mir thread)
{
    qCDebug(QTMIR_MIR_DISPLAYS) << "QtCompositor::stop";
    QMutexLocker lock(&m_runningMutex);

    if (!m_running) {
        return;
    }
    Q_EMIT onStopping(); // blocks

    m_running = false;
}

void QtCompositor::onStopping() // in GUI thread
{
    // Stop Qt's render threads by setting all its windows it obscured. Must
    // block until all windows have their GL contexts released.
    auto renderLoop = QSGRenderLoop::instance();
    for (auto window : renderLoop->windows()) {
        renderLoop->hide(window);
    }
}

void QtCompositor::updateScreens()
{
    qCDebug(QTMIR_MIR_DISPLAYS) << "QtCompositor::updateScreens";
    std::shared_ptr<mg::DisplayConfiguration> displayConfig = m_display->configuration();

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
                    qCDebug(QTMIR_MIR_DISPLAYS) << "Added Screen with id" << output.id.as_value()
                                                << "and geometry" << screen->geometry();
                }
            }
        }
    );

    // Delete any old & unused Screens
    for (auto screen: oldScreenList) {
        qCDebug(QTMIR_MIR_DISPLAYS) << "Removed Screen with id" << static_cast<Screen *>(screen->handle())->m_outputId.as_value()
                                    << "and geometry" << screen->geometry();
        // The screen is automatically removed from Qt's internal list by the QPlatformScreen deconstructor.
        delete screen;
    }
}

Screen* QtCompositor::findScreen(const QList<QScreen*> &list, const mg::DisplayConfigurationOutputId id)
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
