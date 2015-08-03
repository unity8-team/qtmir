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

#ifndef QTMIR_CURSORIMAGEPROVIDER_H
#define QTMIR_CURSORIMAGEPROVIDER_H

#include <QQuickImageProvider>

// xcursor static lib
extern "C"
{
#include <xcursor.h>
}

namespace qtmir
{

class CursorImage {
public:
    CursorImage(XcursorImages *xcursorImages);
    ~CursorImage();

    QImage qimage;
    QPoint hotspot;
    XcursorImages *xcursorImages;
};

class CursorImageProvider : public QQuickImageProvider
{
public:
    CursorImageProvider();
    virtual ~CursorImageProvider();

    static CursorImageProvider *instance() { return m_instance; }


    QImage requestImage(const QString &cursorName, QSize *size, const QSize &requestedSize) override;

    void loadXCursor(XcursorImages *xcursorImages);

    QPoint hotspot(const QString &cursorName);

private:
    void loadCursors();
    QMap<QString, CursorImage*> m_cursors;
    static CursorImageProvider *m_instance;
};

} // namespace qtmir


#endif // QTMIR_CURSORIMAGEPROVIDER_H
