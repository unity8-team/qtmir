// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISSCREEN_H
#define QHYBRISSCREEN_H

#include <qpa/qplatformscreen.h>
#include <QtCore/QTextStream>
#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

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
  EGLSurface surface() const { return m_surface; }

 private:
  void createAndSetPlatformContext() const;
  void createAndSetPlatformContext();

  QRect m_geometry;
  int m_depth;
  QImage::Format m_format;
  SfClient* m_sfClient;
  SfSurface* m_sfSurface;
  QPlatformOpenGLContext* m_platformContext;
  EGLDisplay m_dpy;
  EGLSurface m_surface;
};

QT_END_NAMESPACE

#endif  // QHYBRISSCREEN_H
