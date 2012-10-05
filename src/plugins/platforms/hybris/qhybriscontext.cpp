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
  eglBindAPI(EGL_OPENGL_ES_API);
  eglContext_ = eglCreateContext(eglDisplay_, eglConfig_, EGL_NO_CONTEXT, attribs.constData());
  ASSERT(eglContext_ != EGL_NO_CONTEXT);
  DLOG("created QHybrisContext (this=%p)", this);
}

QHybrisContext::~QHybrisContext() {
  if (eglContext_ != EGL_NO_CONTEXT) {
    eglDestroyContext(eglDisplay_, eglContext_);
    eglContext_ = EGL_NO_CONTEXT;
  }
  DLOG("deleted QHybrisContext");
}

bool QHybrisContext::makeCurrent(QPlatformSurface* surface) {
  DLOG("QHybrisContext::makeCurrent (surface=%p, this=%p)", surface, this);
  DASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);
  eglBindAPI(EGL_OPENGL_ES_API);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(surface);
  QHybrisScreen* screen = static_cast<QHybrisScreen*>(window->screen());
  EGLSurface eglSurface = screen->surface();
  EGLBoolean eglMakeCurrentResult = eglMakeCurrent(
      eglDisplay_, eglSurface, eglSurface, eglContext_);
  DASSERT(eglMakeCurrentResult != EGL_FALSE);

#if defined(QHYBRIS_DEBUG)
  static bool once = true;
  if (once) {
    const char* str = (const char*) glGetString(GL_VENDOR);
    LOG("OpenGL ES vendor: %s", str);
    str = (const char*) glGetString(GL_RENDERER);
    LOG("OpenGL ES renderer: %s", str);
    str = (const char*) glGetString(GL_VERSION);
    LOG("OpenGL ES version: %s", str);
    str = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION);
    LOG("OpenGL ES Shading Language version: %s",str);
    str = (const char*) glGetString(GL_EXTENSIONS);
    LOG("OpenGL ES extensions: %s", str);
    once = false;
  }
#endif
  return true;
}

void QHybrisContext::doneCurrent() {
  DLOG("QHybrisContext::doneCurrent");
  eglBindAPI(EGL_OPENGL_ES_API);
  EGLBoolean eglMakeCurrentResult = eglMakeCurrent(
      eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  DASSERT(eglMakeCurrentResult != EGL_FALSE);
}

void QHybrisContext::swapBuffers(QPlatformSurface* surface) {
  DLOG("QHybrisContext::swapBuffers (surface=%p)", surface);
  eglBindAPI(EGL_OPENGL_ES_API);
  QHybrisWindow* window = static_cast<QHybrisWindow*>(surface);
  QHybrisScreen* screen = static_cast<QHybrisScreen*>(window->screen());
  EGLSurface eglSurface = screen->surface();
  EGLBoolean eglSwapBuffersResult = eglSwapBuffers(eglDisplay_, eglSurface);
  DASSERT(eglSwapBuffersResult != EGL_FALSE);
}

void (*QHybrisContext::getProcAddress(const QByteArray& procName)) () {
  DLOG("QHybrisContext::getProcAddress (procName=%s)", procName.constData());
  eglBindAPI(EGL_OPENGL_ES_API);
  return eglGetProcAddress(procName.constData());
}
