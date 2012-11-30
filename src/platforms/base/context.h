// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBASECONTEXT_H
#define QHYBRISBASECONTEXT_H

#include <qpa/qplatformopenglcontext.h>
#include "screen.h"

class QHybrisBaseContext : public QPlatformOpenGLContext {
 public:
  QHybrisBaseContext(QHybrisBaseScreen* screen);
  ~QHybrisBaseContext();

  // QPlatformOpenGLContext methods.
  QSurfaceFormat format() const { return screen_->surfaceFormat(); }
  void swapBuffers(QPlatformSurface* surface);
  bool makeCurrent(QPlatformSurface* surface);
  void doneCurrent();
  bool isValid() const { return eglContext_ != EGL_NO_CONTEXT; }
  void (*getProcAddress(const QByteArray& procName)) ();
  EGLContext eglContext() const { return eglContext_; }

 private:
  QHybrisBaseScreen* screen_;
  EGLContext eglContext_;
  EGLDisplay eglDisplay_;
};

#endif  //QHYBRISBASECONTEXT_H
