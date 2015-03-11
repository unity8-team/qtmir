/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "surfaceobserver.h"

#include <QMetaObject>

SurfaceObserver::SurfaceObserver()
    : m_listener(nullptr)
    , m_framesAvailable(0)
{
}

void SurfaceObserver::setListener(QObject *listener) // called in Qt Gui thread
{
    m_listenerLock.lockForWrite();
    m_listener = listener;
    m_listenerLock.unlock();

    if (m_framesAvailable > 0) {
        Q_EMIT framesPosted(m_framesAvailable);
    }
}

void SurfaceObserver::frame_posted(int framesAvailable) //called in Mir thread
{
    m_framesAvailable = framesAvailable;

    m_listenerLock.lockForRead();
    if (m_listener) {
        Q_EMIT framesPosted(framesAvailable);
    }
    m_listenerLock.unlock();
}

int SurfaceObserver::framesAvailable() const
{
    return m_framesAvailable;
}
