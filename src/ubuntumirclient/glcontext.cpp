/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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
#include "window.h"
#include "logging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>

static void printOpenGLESConfig() {
  static bool once = true;
  if (once) {
    const char* string = (const char*) glGetString(GL_VENDOR);
    qCDebug(ubuntumirclient, "OpenGL ES vendor: %s", string);
    string = (const char*) glGetString(GL_RENDERER);
    qCDebug(ubuntumirclient, "OpenGL ES renderer: %s", string);
    string = (const char*) glGetString(GL_VERSION);
    qCDebug(ubuntumirclient, "OpenGL ES version: %s", string);
    string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
    qCDebug(ubuntumirclient, "OpenGL ES Shading Language version: %s", string);
    string = (const char*) glGetString(GL_EXTENSIONS);
    qCDebug(ubuntumirclient, "OpenGL ES extensions: %s", string);
    once = false;
  }
}

static EGLenum api_in_use()
{
#ifdef QTUBUNTU_USE_OPENGL
    return EGL_OPENGL_API;
#else
    return EGL_OPENGL_ES_API;
#endif
}

UbuntuOpenGLContext::UbuntuOpenGLContext(UbuntuScreen* screen, UbuntuOpenGLContext* share)
{
    ASSERT(screen != NULL);
    mEglDisplay = screen->eglDisplay();
    mScreen = screen;

    // Create an OpenGL ES 2 context.
    QVector<EGLint> attribs;
    attribs.append(EGL_CONTEXT_CLIENT_VERSION);
    attribs.append(2);
    attribs.append(EGL_NONE);
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);

    mEglContext = eglCreateContext(mEglDisplay, screen->eglConfig(), share ? share->eglContext() : EGL_NO_CONTEXT,
                                   attribs.constData());
    Q_ASSERT(mEglContext != EGL_NO_CONTEXT);
}

UbuntuOpenGLContext::~UbuntuOpenGLContext()
{
    ASSERT(eglDestroyContext(mEglDisplay, mEglContext) == EGL_TRUE);
}

bool UbuntuOpenGLContext::makeCurrent(QPlatformSurface* surface)
{
    Q_ASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);
    EGLSurface eglSurface = static_cast<UbuntuWindow*>(surface)->eglSurface();
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);
    ASSERT(eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, mEglContext) == EGL_TRUE);
    if (ubuntumirclient().isDebugEnabled()) {
        printOpenGLESConfig();
    }
    return true;
}

void UbuntuOpenGLContext::doneCurrent()
{
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);
    ASSERT(eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE);
}

void UbuntuOpenGLContext::swapBuffers(QPlatformSurface* surface)
{
    UbuntuWindow *ubuntuWindow = static_cast<UbuntuWindow*>(surface);

    EGLSurface eglSurface = ubuntuWindow->eglSurface();
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);
    ASSERT(eglSwapBuffers(mEglDisplay, eglSurface) == EGL_TRUE);

    ubuntuWindow->onSwapBuffersDone();
}

void (*UbuntuOpenGLContext::getProcAddress(const QByteArray& procName)) ()
{
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);
    return eglGetProcAddress(procName.constData());
}
