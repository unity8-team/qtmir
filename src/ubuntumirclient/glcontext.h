/*
 * Copyright (C) 2014,2016 Canonical, Ltd.
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

#ifndef UBUNTU_OPENGL_CONTEXT_H
#define UBUNTU_OPENGL_CONTEXT_H

#include <qpa/qplatformopenglcontext.h>
#include <private/qeglplatformcontext_p.h>

#include <EGL/egl.h>

class UbuntuOpenGLContext : public QEGLPlatformContext
{
public:
    UbuntuOpenGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share,
                        EGLDisplay display, EGLConfig *config = 0);

    // QEGLPlatformContext methods.
    void swapBuffers(QPlatformSurface *surface) override;
    bool makeCurrent(QPlatformSurface *surface) override;

protected:
    EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface *surface) override;
};

#endif // UBUNTU_OPENGL_CONTEXT_H
