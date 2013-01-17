// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASECONTEXT_H
#define QUBUNTUBASECONTEXT_H

#include <qpa/qplatformopenglcontext.h>
#include "screen.h"

class QUbuntuBaseContext : public QPlatformOpenGLContext {
 public:
  QUbuntuBaseContext(QUbuntuBaseScreen* screen);
  ~QUbuntuBaseContext();

  // QPlatformOpenGLContext methods.
  QSurfaceFormat format() const { return screen_->surfaceFormat(); }
  void swapBuffers(QPlatformSurface* surface);
  bool makeCurrent(QPlatformSurface* surface);
  void doneCurrent();
  bool isValid() const { return eglContext_ != EGL_NO_CONTEXT; }
  void (*getProcAddress(const QByteArray& procName)) ();
  EGLContext eglContext() const { return eglContext_; }

 private:
  QUbuntuBaseScreen* screen_;
  EGLContext eglContext_;
  EGLDisplay eglDisplay_;
};

#endif  //QUBUNTUBASECONTEXT_H
