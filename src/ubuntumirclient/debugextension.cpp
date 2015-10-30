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
 */

#include "debugextension.h"

#include <QDebug>

// mir client debug
#include "mir_toolkit/debug/surface.h"

UbuntuDebugExtension::UbuntuDebugExtension()
    : m_mirclientDebug(QStringLiteral("mirclient-debug-extension"))
    , m_mapper(nullptr)
{
    qDebug() << "NOTICE: Loading mirclient-debug-extension";
    m_mapper = (MapperPrototype) m_mirclientDebug.resolve("mir_debug_surface_coords_to_screen");

    if (m_mirclientDebug.isLoaded()) {
        qWarning() << "ERROR: mirclient-debug-extension failed to load:" << m_mirclientDebug.errorString();
    } else if (!m_mapper) {
        qWarning() << "ERROR: unable to find required symbols in mirclient-debug-extension:"
                   << m_mirclientDebug.errorString();
    }
}

QPoint UbuntuDebugExtension::mapSurfacePointToScreen(MirSurface *surface, const QPoint &point)
{
    if (!m_mapper) {
        return point;
    }

    QPoint mappedPoint;
    bool status = m_mapper(surface, point.x(), point.y(), &mappedPoint.rx(), &mappedPoint.ry());
    if (status) {
        return mappedPoint;
    } else {
        return point;
    }
}
