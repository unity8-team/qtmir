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

#include "mousepointer.h"
#include "cursorimageprovider.h"
#include "mirsingleton.h"

// QPA mirserver
#include <cursor.h>

#include <QQuickWindow>
#include <QGuiApplication>

#include <qpa/qwindowsysteminterface.h>

using namespace qtmir;

MousePointer::MousePointer(QQuickItem *parent)
    : MousePointerInterface(parent)
    , m_themeName("default")
    , m_hotspotX(0)
    , m_hotspotY(0)
{
    connect(Mir::instance(), &Mir::cursorNameChanged, this, &MousePointer::updateCursorName);
    updateCursorName();
}

void MousePointer::handleMouseEvent(ulong timestamp, QPointF movement, Qt::MouseButton buttons,
        Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(timestamp);
    Q_UNUSED(buttons);
    Q_UNUSED(modifiers);

    if (!parentItem()) {
        return;
    }

    qreal newX = x() + movement.x();
    if (newX < 0) {
        newX = 0;
    } else if (newX > parentItem()->width()) {
        newX = parentItem()->width();
    }
    setX(newX);

    qreal newY = y() + movement.y();
    if (newY < 0) {
        newY = 0;
    } else if (newY > parentItem()->height()) {
        newY = parentItem()->height();
    }
    setY(newY);

    QPointF scenePosition = mapToItem(nullptr, QPointF(0, 0));
    QWindowSystemInterface::handleMouseEvent(window(), timestamp, scenePosition /*local*/, scenePosition /*global*/,
        buttons, modifiers);
}

void MousePointer::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemSceneChange) {
        registerWindow(value.window);
    }
}

void MousePointer::registerWindow(QWindow *window)
{
    if (m_registeredWindow && window != m_registeredWindow) {
        Cursor *previousCursor = static_cast<qtmir::Cursor*>(m_registeredWindow->screen()->handle()->cursor());
        previousCursor->setMousePointer(nullptr);
    }

    m_registeredWindow = window;

    if (m_registeredWindow) {
        Cursor *cursor = static_cast<qtmir::Cursor*>(window->screen()->handle()->cursor());
        cursor->setMousePointer(this);
    }
}

void MousePointer::setQtCursorName(const QString &qtCursorName)
{
    if (qtCursorName != m_qtCursorName) {
        m_qtCursorName = qtCursorName;
        updateCursorName();
    }
}

void MousePointer::updateCursorName()
{
    if (Mir::instance()->cursorName().isEmpty()) {
        if (m_qtCursorName.isEmpty()) {
            setCursorName("left_ptr");
        } else {
            setCursorName(m_qtCursorName);
        }
    } else {
        setCursorName(Mir::instance()->cursorName());
    }
}

void MousePointer::setCursorName(const QString &cursorName)
{
    if (cursorName != m_cursorName) {
        m_cursorName = cursorName;
        Q_EMIT cursorNameChanged(m_cursorName);
        updateHotspot();
    }
}

void MousePointer::updateHotspot()
{
    QPoint newHotspot = CursorImageProvider::instance()->hotspot(m_themeName, m_cursorName);

    if (m_hotspotX != newHotspot.x()) {
        m_hotspotX = newHotspot.x();
        Q_EMIT hotspotXChanged(m_hotspotX);
    }

    if (m_hotspotY != newHotspot.y()) {
        m_hotspotY = newHotspot.y();
        Q_EMIT hotspotYChanged(m_hotspotY);
    }
}

void MousePointer::setThemeName(const QString &themeName)
{
    if (m_themeName != themeName) {
        m_themeName = themeName;
        Q_EMIT themeNameChanged(m_themeName);
    }
}
