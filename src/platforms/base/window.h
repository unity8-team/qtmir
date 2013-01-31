// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASEWINDOW_H
#define QUBUNTUBASEWINDOW_H

#include <qpa/qplatformwindow.h>
#include <EGL/egl.h>

class QUbuntuBaseScreen;

class QUbuntuBaseWindow : public QPlatformWindow {
 public:
  QUbuntuBaseWindow(QWindow* w, QUbuntuBaseScreen* screen);
  ~QUbuntuBaseWindow();

  // QPlatformWindow methods.
  WId winId() const { return id_; }

  // New methods.
  void createSurface(EGLNativeWindowType nativeWindow);
  EGLSurface eglSurface() const { return eglSurface_; }

 private:
  QUbuntuBaseScreen* screen_;
  EGLSurface eglSurface_;
  WId id_;
};

#endif  // QUBUNTUBASEWINDOW_H
