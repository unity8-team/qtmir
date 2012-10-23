// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "context.h"
#include "window.h"
#include "screen.h"
#include "logging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#if defined(QHYBRIS_DEBUG)
static void printOpenGLESConfig() {
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
}
#endif

QHybrisBaseContext::QHybrisBaseContext(QHybrisBaseScreen* screen) {
  DASSERT(screen != NULL);
  eglDisplay_ = screen->eglDisplay();
  screen_ = screen;

  // Create an OpenGL ES 2 context.
  QVector<EGLint> attribs;
  attribs.append(EGL_CONTEXT_CLIENT_VERSION);
  attribs.append(2);
  attribs.append(EGL_NONE);
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT((eglContext_ = eglCreateContext(
      eglDisplay_, screen->eglConfig(), EGL_NO_CONTEXT, attribs.constData())) != EGL_NO_CONTEXT);

  DLOG("QHybrisBaseContext::QHybrisBaseContext (this=%p)", this);
}

QHybrisBaseContext::~QHybrisBaseContext() {
  DLOG("QHybrisBaseContext::~QHybrisBaseContext");
  ASSERT(eglDestroyContext(eglDisplay_, eglContext_) == EGL_TRUE);
}

bool QHybrisBaseContext::makeCurrent(QPlatformSurface* surface) {
  // DLOG("QHybrisBaseContext::makeCurrent (this=%p, surface=%p)", this, surface);
  DASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);
  EGLSurface eglSurface = static_cast<QHybrisBaseWindow*>(surface)->eglSurface();
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_) == EGL_TRUE);
  printOpenGLESConfig();
#endif
  return true;
}

void QHybrisBaseContext::doneCurrent() {
  DLOG("QHybrisBaseContext::doneCurrent (this=%p)", this);
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE);
#endif
}

void QHybrisBaseContext::swapBuffers(QPlatformSurface* surface) {
  // DLOG("QHybrisBaseContext::swapBuffers (this=%p, surface=%p)", this, surface);
  EGLSurface eglSurface = static_cast<QHybrisBaseWindow*>(surface)->eglSurface();
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglSwapBuffers(eglDisplay_, eglSurface);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglSwapBuffers(eglDisplay_, eglSurface) == EGL_TRUE);
#endif
}

void (*QHybrisBaseContext::getProcAddress(const QByteArray& procName)) () {
  DLOG("QHybrisBaseContext::getProcAddress (this=%p, procName=%s)", this, procName.constData());
#if !defined(QHYBRIS_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
#endif
  return eglGetProcAddress(procName.constData());
}

QSurfaceFormat QHybrisBaseContext::format() const {
  DLOG("QHybrisBaseContext::format (this=%p)", this);
  DASSERT(screen_ != NULL);
  return screen_->surfaceFormat();
}
