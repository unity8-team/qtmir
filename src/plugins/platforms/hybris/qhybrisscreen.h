// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISSCREEN_H
#define QHYBRISSCREEN_H

#include <qpa/qplatformscreen.h>
#include <QSurfaceFormat>
#include <EGL/egl.h>

struct SfClient;

class QHybrisScreen : public QPlatformScreen {
 public:
  QHybrisScreen();
  ~QHybrisScreen();

  QRect geometry() const;
  int depth() const;
  QImage::Format format() const;

  SfClient* sfClient() const { return sfClient_; }
  EGLDisplay eglDisplay() const { return eglDisplay_; }
  EGLConfig eglConfig() const { return eglConfig_; }
  QSurfaceFormat surfaceFormat() const { return surfaceFormat_; }

 private:
  SfClient* sfClient_;
  EGLDisplay eglDisplay_;
  QRect geometry_;
  QSurfaceFormat surfaceFormat_;
  EGLConfig eglConfig_;
  QImage::Format format_;
  int depth_;
};

#endif  // QHYBRISSCREEN_H
