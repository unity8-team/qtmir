/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#include "glcontext.h"
#include "logging.h"
#include "offscreensurface.h"
#include "window.h"

#include <QOpenGLFramebufferObject>
#include <QtPlatformSupport/private/qeglconvenience_p.h>

namespace {

void printEglConfig(EGLDisplay display, EGLConfig config) {
    Q_ASSERT(display != EGL_NO_DISPLAY);
    Q_ASSERT(config != nullptr);

    const char* string = eglQueryString(display, EGL_VENDOR);
    qCDebug(ubuntumirclient, "EGL vendor: %s", string);

    string = eglQueryString(display, EGL_VERSION);
    qCDebug(ubuntumirclient, "EGL version: %s", string);

    string = eglQueryString(display, EGL_EXTENSIONS);
    qCDebug(ubuntumirclient, "EGL extensions: %s", string);

    qCDebug(ubuntumirclient, "EGL configuration attibutes:");
    q_printEglConfig(display, config);
}

} // anonymous namespace

UbuntuOpenGLContext::UbuntuOpenGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share,
                                         EGLDisplay display, EGLConfig *config)
    : QEGLPlatformContext(format, share, display, config)
{
    if (ubuntumirclient().isDebugEnabled()) {
        printEglConfig(display, config);
    }
}

bool UbuntuOpenGLContext::makeCurrent(QPlatformSurface* surface)
{
    Q_ASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);

    if (surface->surface()->surfaceClass() == QSurface::Offscreen) {
        auto offscreen = static_cast<UbuntuOffscreenSurface *>(surface);
        if (!offscreen->buffer()) {
            auto buffer = new QOpenGLFramebufferObject(surface->surface()->size());
            offscreen->setBuffer(buffer);
        }
        return offscreen->buffer()->bind();
    } else {
        return QEGLPlatformContext::makeCurrent(surface);
    }
}

// Following method used internally in the base class QEGLPlatformContext to access
// the egl surface of a QPlatformSurface/UbuntuWindow
EGLSurface UbuntuOpenGLContext::eglSurfaceForPlatformSurface(QPlatformSurface *surface)
{
    auto ubuntuWindow = static_cast<UbuntuWindow*>(surface);
    return ubuntuWindow->eglSurface();
}

void UbuntuOpenGLContext::swapBuffers(QPlatformSurface* surface)
{
    QEGLPlatformContext::swapBuffers(surface);

    // notify window on swap completion
    auto ubuntuWindow = static_cast<UbuntuWindow*>(surface);
    ubuntuWindow->onSwapBuffersDone();
}
