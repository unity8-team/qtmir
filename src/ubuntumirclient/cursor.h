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

#ifndef UBUNTU_CURSOR_H
#define UBUNTU_CURSOR_H

#include <qpa/qplatformcursor.h>

#include <QMap>
#include <QByteArray>

struct MirConnection;
struct MirSurface;

class UbuntuCursor : public QPlatformCursor
{
public:
    UbuntuCursor(MirConnection *connection);
    void changeCursor(QCursor *windowCursor, QWindow *window) override;
private:
    void configureMirCursorWithPixmapQCursor(MirSurface *surface, QCursor &cursor);
    void applyDefaultCursorConfiguration(MirSurface *surface);
    QMap<int, QByteArray> mShapeToCursorName;
    MirConnection *mConnection;
};

#endif // UBUNTU_CURSOR_H
