// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "context.h"
#include "window.h"
#include "logging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#if !defined(QT_NO_DEBUG)
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

QUbuntuBaseContext::QUbuntuBaseContext(QUbuntuBaseScreen* screen) {
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

  DLOG("QUbuntuBaseContext::QUbuntuBaseContext (this=%p, screen=%p)", this, screen);
}

QUbuntuBaseContext::~QUbuntuBaseContext() {
  DLOG("QUbuntuBaseContext::~QUbuntuBaseContext");
  ASSERT(eglDestroyContext(eglDisplay_, eglContext_) == EGL_TRUE);
}

bool QUbuntuBaseContext::makeCurrent(QPlatformSurface* surface) {
  // DLOG("QUbuntuBaseContext::makeCurrent (this=%p, surface=%p)", this, surface);
  DASSERT(surface->surface()->surfaceType() == QSurface::OpenGLSurface);
  EGLSurface eglSurface = static_cast<QUbuntuBaseWindow*>(surface)->eglSurface();
#if defined(QT_NO_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, eglSurface, eglSurface, eglContext_) == EGL_TRUE);
  printOpenGLESConfig();
#endif
  return true;
}

void QUbuntuBaseContext::doneCurrent() {
  DLOG("QUbuntuBaseContext::doneCurrent (this=%p)", this);
#if defined(QT_NO_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE);
#endif
}

void QUbuntuBaseContext::swapBuffers(QPlatformSurface* surface) {
  // DLOG("QUbuntuBaseContext::swapBuffers (this=%p, surface=%p)", this, surface);
  EGLSurface eglSurface = static_cast<QUbuntuBaseWindow*>(surface)->eglSurface();
#if defined(QT_NO_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
  eglSwapBuffers(eglDisplay_, eglSurface);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT(eglSwapBuffers(eglDisplay_, eglSurface) == EGL_TRUE);
#endif
}

void (*QUbuntuBaseContext::getProcAddress(const QByteArray& procName)) () {
  DLOG("QUbuntuBaseContext::getProcAddress (this=%p, procName=%s)", this, procName.constData());
#if defined(QT_NO_DEBUG)
  eglBindAPI(EGL_OPENGL_ES_API);
#else
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
#endif
  return eglGetProcAddress(procName.constData());
}
