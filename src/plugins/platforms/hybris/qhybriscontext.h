// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISCONTEXT_H
#define QHYBRISCONTEXT_H

#include <qpa/qplatformwindow.h>
#include <qpa/qplatformopenglcontext.h>
#include <EGL/egl.h>

class QHybrisContext : public QPlatformOpenGLContext {
 public:
  QHybrisContext(const QSurfaceFormat& format, EGLDisplay display);
  ~QHybrisContext();
  bool makeCurrent(QPlatformSurface* surface);
  void doneCurrent();
  void swapBuffers(QPlatformSurface* surface);
  void (*getProcAddress(const QByteArray& procName)) ();
  bool isValid() const { return eglContext_ != EGL_NO_CONTEXT; }
  QSurfaceFormat format() const { return format_; }
  EGLContext eglContext() const { return eglContext_; }
  EGLDisplay eglDisplay() const { return eglDisplay_; }
  EGLConfig eglConfig() const { return eglConfig_; }

 private:
  EGLContext eglContext_;
  EGLDisplay eglDisplay_;
  EGLConfig eglConfig_;
  const QSurfaceFormat format_;
};

#endif  //QHYBRISCONTEXT_H
