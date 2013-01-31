// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUBASESCREEN_H
#define QUBUNTUBASESCREEN_H

#include <qpa/qplatformscreen.h>
#include <QSurfaceFormat>
#include <EGL/egl.h>

class QUbuntuBaseScreen : public QPlatformScreen {
 public:
  QUbuntuBaseScreen();
  ~QUbuntuBaseScreen();

  // QPlatformScreen methods.
  QImage::Format format() const { return format_; }
  int depth() const { return depth_; }

  // New methods.
  QSurfaceFormat surfaceFormat() const { return surfaceFormat_; }
  EGLDisplay eglDisplay() const { return eglDisplay_; }
  EGLConfig eglConfig() const { return eglConfig_; }

 private:
  QImage::Format format_;
  int depth_;
  QSurfaceFormat surfaceFormat_;
  EGLDisplay eglDisplay_;
  EGLConfig eglConfig_;
};

#endif  // QUBUNTUBASESCREEN_H
