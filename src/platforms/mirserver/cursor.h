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
 */

#ifndef QTMIR_CURSOR_H
#define QTMIR_CURSOR_H

#include <QMutex>
#include <QPointer>
#include <qpa/qplatformcursor.h>

namespace qtmir {

class MousePointerInterface;

class Cursor : public QPlatformCursor
{
public:
    Cursor();

    void changeCursor(QCursor *windowCursor, QWindow *window) override;

    // Called from Qt's GUI thread
    void setMousePointer(MousePointerInterface *mousePointer);

    // Called form Mir input thread
    void handleMouseEvent(ulong timestamp, QPointF movement, Qt::MouseButton buttons,
            Qt::KeyboardModifiers modifiers);

private:
    void loadCursors();

    QMutex m_mutex;
    QPointer<MousePointerInterface> m_mousePointer;
    QMap<int,QString> m_shapeToCursorName;
};

} // namespace qtmir

#endif // QTMIR_CURSOR_H
