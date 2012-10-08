// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISSCREEN_H
#define QHYBRISSCREEN_H

#include <qpa/qplatformscreen.h>
#include <EGL/egl.h>

struct SfClient;
struct SfSurface;
class QPlatformOpenGLContext;

class QHybrisScreen : public QPlatformScreen {
 public:
  QHybrisScreen();
  ~QHybrisScreen();

  QRect geometry() const;
  int depth() const;
  QImage::Format format() const;
  QPlatformOpenGLContext* platformContext() const;
  EGLSurface surface() const { return eglSurface_; }

 private:
  void createAndSetPlatformContext() const;
  void createAndSetPlatformContext();

  QRect geometry_;
  int depth_;
  QImage::Format format_;
  SfClient* sfClient_;
  SfSurface* sfSurface_;
  QPlatformOpenGLContext* platformContext_;
  EGLDisplay eglDisplay_;
  EGLSurface eglSurface_;
};

#endif  // QHYBRISSCREEN_H
