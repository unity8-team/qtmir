// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBASEWINDOW_H
#define QHYBRISBASEWINDOW_H

#include <qpa/qplatformwindow.h>
#include <EGL/egl.h>

class QHybrisBaseScreen;

class QHybrisBaseWindow : public QPlatformWindow {
 public:
  QHybrisBaseWindow(QWindow* w, QHybrisBaseScreen* screen);
  ~QHybrisBaseWindow();

  // QPlatformWindow methods.
  WId winId() const { return id_; }

  // New methods.
  void createSurface(EGLNativeWindowType nativeWindow);
  EGLSurface eglSurface() const { return eglSurface_; }

 private:
  QHybrisBaseScreen* screen_;
  EGLSurface eglSurface_;
  WId id_;
};

#endif  // QHYBRISBASEWINDOW_H
