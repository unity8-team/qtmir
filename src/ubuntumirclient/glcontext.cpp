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
#include <QtGui/private/qopenglcontext_p.h>

namespace {

void printOpenGLESConfig() {
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

void printEglConfig(EGLDisplay display, EGLConfig config) {
    Q_ASSERT(display != EGL_NO_DISPLAY);
    Q_ASSERT(config != nullptr);

    static const struct { const EGLint attrib; const char* name; } kAttribs[] = {
        { EGL_BUFFER_SIZE, "EGL_BUFFER_SIZE" },
        { EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE" },
        { EGL_BLUE_SIZE, "EGL_BLUE_SIZE" },
        { EGL_GREEN_SIZE, "EGL_GREEN_SIZE" },
        { EGL_RED_SIZE, "EGL_RED_SIZE" },
        { EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE" },
        { EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE" },
        { EGL_CONFIG_CAVEAT, "EGL_CONFIG_CAVEAT" },
        { EGL_CONFIG_ID, "EGL_CONFIG_ID" },
        { EGL_LEVEL, "EGL_LEVEL" },
        { EGL_MAX_PBUFFER_HEIGHT, "EGL_MAX_PBUFFER_HEIGHT" },
        { EGL_MAX_PBUFFER_PIXELS, "EGL_MAX_PBUFFER_PIXELS" },
        { EGL_MAX_PBUFFER_WIDTH, "EGL_MAX_PBUFFER_WIDTH" },
        { EGL_NATIVE_RENDERABLE, "EGL_NATIVE_RENDERABLE" },
        { EGL_NATIVE_VISUAL_ID, "EGL_NATIVE_VISUAL_ID" },
        { EGL_NATIVE_VISUAL_TYPE, "EGL_NATIVE_VISUAL_TYPE" },
        { EGL_SAMPLES, "EGL_SAMPLES" },
        { EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS" },
        { EGL_SURFACE_TYPE, "EGL_SURFACE_TYPE" },
        { EGL_TRANSPARENT_TYPE, "EGL_TRANSPARENT_TYPE" },
        { EGL_TRANSPARENT_BLUE_VALUE, "EGL_TRANSPARENT_BLUE_VALUE" },
        { EGL_TRANSPARENT_GREEN_VALUE, "EGL_TRANSPARENT_GREEN_VALUE" },
        { EGL_TRANSPARENT_RED_VALUE, "EGL_TRANSPARENT_RED_VALUE" },
        { EGL_BIND_TO_TEXTURE_RGB, "EGL_BIND_TO_TEXTURE_RGB" },
        { EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA" },
        { EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL" },
        { EGL_MAX_SWAP_INTERVAL, "EGL_MAX_SWAP_INTERVAL" },
        { -1, NULL }
    };
    const char* string = eglQueryString(display, EGL_VENDOR);
    qCDebug(ubuntumirclient, "EGL vendor: %s", string);

    string = eglQueryString(display, EGL_VERSION);
    qCDebug(ubuntumirclient, "EGL version: %s", string);

    string = eglQueryString(display, EGL_EXTENSIONS);
    qCDebug(ubuntumirclient, "EGL extensions: %s", string);

    qCDebug(ubuntumirclient, "EGL configuration attibutes:");
    for (int index = 0; kAttribs[index].attrib != -1; index++) {
        EGLint value;
        if (eglGetConfigAttrib(display, config, kAttribs[index].attrib, &value))
            qCDebug(ubuntumirclient, "  %s: %d", kAttribs[index].name, static_cast<int>(value));
    }
}

QString eglErrorToString(EGLint errorNumber)
{
    #define EGL_ERROR_CASE(error) case error: return QString(#error);

    switch (errorNumber) {
        EGL_ERROR_CASE(EGL_SUCCESS)
        EGL_ERROR_CASE(EGL_NOT_INITIALIZED)
        EGL_ERROR_CASE(EGL_BAD_ACCESS)
        EGL_ERROR_CASE(EGL_BAD_ALLOC)
        EGL_ERROR_CASE(EGL_BAD_ATTRIBUTE)
        EGL_ERROR_CASE(EGL_BAD_CONTEXT)
        EGL_ERROR_CASE(EGL_BAD_CONFIG)
        EGL_ERROR_CASE(EGL_BAD_CURRENT_SURFACE)
        EGL_ERROR_CASE(EGL_BAD_DISPLAY)
        EGL_ERROR_CASE(EGL_BAD_SURFACE)
        EGL_ERROR_CASE(EGL_BAD_MATCH)
        EGL_ERROR_CASE(EGL_BAD_PARAMETER)
        EGL_ERROR_CASE(EGL_BAD_NATIVE_PIXMAP)
        EGL_ERROR_CASE(EGL_BAD_NATIVE_WINDOW)
        EGL_ERROR_CASE(EGL_CONTEXT_LOST)
        default:
            return QString("?");
    }

    #undef EGL_ERROR_CASE
}

EGLenum api_in_use()
{
    #ifdef QTUBUNTU_USE_OPENGL
    return EGL_OPENGL_API;
    #else
    return EGL_OPENGL_ES_API;
    #endif
}

const int kSwapInterval = 1;

int qGetEnvIntValue(const char *varName, bool *ok)
{
    return qgetenv(varName).toInt(ok);
}

} // anonymous namespace

UbuntuOpenGLContext::UbuntuOpenGLContext(const QSurfaceFormat &surfaceFormat, UbuntuOpenGLContext *share,
                                         EGLDisplay display, EGLConfig config)
    : mSurfaceFormat(surfaceFormat)
    , mEglDisplay(display)
{
    // Create an OpenGL ES 2 context.
    QVector<EGLint> attribs;
    attribs.append(EGL_CONTEXT_CLIENT_VERSION);
    attribs.append(2);
    attribs.append(EGL_NONE);
    ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);

    if (ubuntumirclient().isDebugEnabled()) {
        printEglConfig(mEglDisplay, config);
    }

    // Set vblank swap interval.
    bool ok;
    int swapInterval = qGetEnvIntValue("QTUBUNTU_SWAPINTERVAL", &ok);
    if (!ok)
        swapInterval = kSwapInterval;

    qCDebug(ubuntumirclient, "Setting swap interval to %d", swapInterval);
    eglSwapInterval(mEglDisplay, swapInterval);

    mEglContext = eglCreateContext(mEglDisplay, config, share ? share->eglContext() : EGL_NO_CONTEXT,
                                   attribs.constData());

    Q_ASSERT(mEglContext != EGL_NO_CONTEXT);
}

UbuntuOpenGLContext::~UbuntuOpenGLContext()
{
    ASSERT(eglDestroyContext(mEglDisplay, mEglContext) == EGL_TRUE);
}

static bool needsFBOReadBackWorkaround()
{
    static bool set = false;
    static bool needsWorkaround = false;

    if (!set) {
        const char *rendererString = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        needsWorkaround = qstrncmp(rendererString, "Mali-400", 8) == 0;
        set = true;
    }

    return needsWorkaround;
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
        EGLSurface eglSurface = static_cast<UbuntuWindow*>(surface)->eglSurface();
        ASSERT(eglBindAPI(api_in_use()) == EGL_TRUE);

        EGLBoolean result = eglMakeCurrent(mEglDisplay, eglSurface, eglSurface, mEglContext);
        if (result == EGL_FALSE) {
            qCCritical(ubuntumirclient, "eglMakeCurrent() failed with %s",
                    qPrintable(eglErrorToString(eglGetError())));
            return false;
        }

        QOpenGLContextPrivate *ctx_d = QOpenGLContextPrivate::get(context());
        if (!ctx_d->workaround_brokenFBOReadBack && needsFBOReadBackWorkaround()) {
            ctx_d->workaround_brokenFBOReadBack = true;
        }

        if (ubuntumirclient().isDebugEnabled()) {
            printOpenGLESConfig();
        }
        return true;
    }
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
