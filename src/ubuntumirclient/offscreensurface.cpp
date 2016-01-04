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

#include "offscreensurface.h"

#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>

UbuntuOffscreenSurface::UbuntuOffscreenSurface(QOffscreenSurface *offscreenSurface)
    : QPlatformOffscreenSurface(offscreenSurface)
    , m_buffer(nullptr)
    , m_format(offscreenSurface->requestedFormat())
{
}

QSurfaceFormat UbuntuOffscreenSurface::format() const
{
    return m_format;
}

bool UbuntuOffscreenSurface::isValid() const
{
    return m_buffer != nullptr && m_buffer->isValid();
}

QOpenGLFramebufferObject* UbuntuOffscreenSurface::buffer() const
{
    return m_buffer;
}

void UbuntuOffscreenSurface::setBuffer(QOpenGLFramebufferObject *buffer)
{
    m_buffer = buffer;
}
