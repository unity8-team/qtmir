// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISCONTEXT_H
#define QHYBRISCONTEXT_H

#include <qpa/qplatformopenglcontext.h>
#include <EGL/egl.h>

class QHybrisScreen;

class QHybrisContext : public QPlatformOpenGLContext {
 public:
  QHybrisContext(QHybrisScreen* screen);
  ~QHybrisContext();

  QSurfaceFormat format() const;
  void swapBuffers(QPlatformSurface* surface);
  bool makeCurrent(QPlatformSurface* surface);
  void doneCurrent();
  bool isValid() const { return eglContext_ != EGL_NO_CONTEXT; }
  void (*getProcAddress(const QByteArray& procName)) ();
  EGLContext eglContext() const { return eglContext_; }

 private:
  QHybrisScreen* screen_;
  EGLContext eglContext_;
  EGLDisplay eglDisplay_;
};

#endif  //QHYBRISCONTEXT_H
