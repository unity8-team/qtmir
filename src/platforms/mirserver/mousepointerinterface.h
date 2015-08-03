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

#ifndef QTMIR_MOUSEPOINTERINTERFACE_H
#define QTMIR_MOUSEPOINTERINTERFACE_H

#include <QQuickItem>

namespace qtmir {

class MousePointerInterface : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QString cursorName READ cursorName NOTIFY cursorNameChanged)
    Q_PROPERTY(qreal hotspotX READ hotspotX NOTIFY hotspotXChanged)
    Q_PROPERTY(qreal hotspotY READ hotspotY NOTIFY hotspotYChanged)
public:
    MousePointerInterface(QQuickItem *parent = nullptr) : QQuickItem(parent) {}

    virtual void setQtCursorName(const QString &cursorName) = 0;

    virtual QString cursorName() const = 0;

    virtual qreal hotspotX() const = 0;
    virtual qreal hotspotY() const = 0;

Q_SIGNALS:
    void cursorNameChanged(QString name);
    void hotspotXChanged(qreal value);
    void hotspotYChanged(qreal value);

public Q_SLOTS:
    virtual void handleMouseEvent(ulong timestamp, QPointF movement, Qt::MouseButton buttons,
            Qt::KeyboardModifiers modifiers) = 0;

};

}

#endif // QTMIR_MOUSEPOINTERINTERFACE_H
