/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#ifndef UBUNTU_DEBUG_EXTENSION_H
#define UBUNTU_DEBUG_EXTENSION_H

#include <QPoint>
#include <QLibrary>
struct MirSurface;

typedef bool (*MapperPrototype)(MirSurface* surface, int x, int y, int* screenX, int* screenY);


class UbuntuDebugExtension
{
public:
    UbuntuDebugExtension();

    QPoint mapSurfacePointToScreen(MirSurface *, const QPoint &point);

private:
    QLibrary m_mirclientDebug;
    MapperPrototype m_mapper;
};

#endif // UBUNTU_DEBUG_EXTENSION_H
