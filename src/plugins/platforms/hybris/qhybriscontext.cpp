// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybriscontext.h"
#include "qhybriswindow.h"
#include "qhybrisscreen.h"
#include "qhybrislogging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>

QHybrisContext::QHybrisContext(const QSurfaceFormat &format, EGLDisplay display)
    : eglDisplay_(display)
    , eglConfig_(q_configFromGLFormat(display, format, true))
    , format_(q_glFormatFromConfig(display, eglConfig_)) {
  QVector<EGLint> attribs;
  attribs.append(EGL_CONTEXT_CLIENT_VERSION);
  attribs.append(format.majorVersion());
  attribs.append(EGL_NONE);
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  eglContext_ = eglCreateContext(eglDisplay_, eglConfig_, EGL_NO_CONTEXT, attribs.constData());
  ASSERT(eglContext_ != EGL_NO_CONTEXT);
  DLOG("QHybrisContext::QHybrisContext (this=%p)", this);
}

QHybrisContext::~QHybrisContext() {
  DLOG("QHybrisContext::~QHybrisContext");
  ASSERT(eglContext_ != EGL_NO_CONTEXT);
  ASSERT(eglDestroyContext(eglDisplay_, eglContext_) == EGL_TRUE);
}

bool QHybrisContext::makeCurrent(QPlatformSurface* surface) {
  DLOG("QHybrisContext::makeCurrent (this=%p, surface=%p)", this, surface);
  DASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(surface);
  QHybrisScreen* screen = static_cast<QHybrisScreen*>(window->screen());
  EGLSurface eglSurface = screen->surface();
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_) == EGL_TRUE);
  static bool once = true;
  if (once) {
    const char* string = (const char*) glGetString(GL_VENDOR);
    LOG("OpenGL ES vendor: %s", string);
    string = (const char*) glGetString(GL_RENDERER);
    LOG("OpenGL ES renderer: %s", string);
    string = (const char*) glGetString(GL_VERSION);
    LOG("OpenGL ES version: %s", string);
    string = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
    LOG("OpenGL ES Shading Language version: %s", string);
    string = (const char*) glGetString(GL_EXTENSIONS);
    LOG("OpenGL ES extensions: %s", string);
    once = false;
  }
#endif
  return true;
}

void QHybrisContext::doneCurrent() {
  DLOG("QHybrisContext::doneCurrent (this=%p)", this);
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE);
#endif
}

void QHybrisContext::swapBuffers(QPlatformSurface* surface) {
  DLOG("QHybrisContext::swapBuffers (this=%p, surface=%p)", this, surface);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(surface);
  QHybrisScreen* screen = static_cast<QHybrisScreen*>(window->screen());
  EGLSurface eglSurface = screen->surface();
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglSwapBuffers(eglDisplay_, eglSurface);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglSwapBuffers(eglDisplay_, eglSurface) == EGL_TRUE);
#endif
}

void (*QHybrisContext::getProcAddress(const QByteArray& procName)) () {
  DLOG("QHybrisContext::getProcAddress (this=%p, procName=%s)", this, procName.constData());
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
#endif
  return eglGetProcAddress(procName.constData());
}
