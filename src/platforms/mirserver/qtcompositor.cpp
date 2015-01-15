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
#include "mirserverintegration.h"
#include "logging.h"

// Mir
#include <mir/graphics/display.h>


// Lives in a Mir thread
QtCompositor::QtCompositor()
    : m_running(false)
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::QtCompositor";
}

QtCompositor::~QtCompositor()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::~QtCompositor";
    stop();
}

void QtCompositor::start()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::start";
    QMutexLocker lock(&m_runningMutex);
    if (m_running) {
        return;
    }
    m_running = true;

    Q_EMIT starting(); // blocks
}

void QtCompositor::stop()
{
    qCDebug(QTMIR_SCREENS) << "QtCompositor::stop";
    QMutexLocker lock(&m_runningMutex);

    if (!m_running) {
        return;
    }
    Q_EMIT stopping(); // blocks

    m_running = false;
}
