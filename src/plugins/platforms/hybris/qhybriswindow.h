// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISWINDOW_H
#define QHYBRISWINDOW_H

#include <qpa/qplatformwindow.h>
#include <EGL/egl.h>

struct SfClient;
struct SfSurface;
class QHybrisScreen;

class QHybrisWindow : public QPlatformWindow {
 public:
  QHybrisWindow(QWindow* w, QHybrisScreen* screen);
  ~QHybrisWindow();

  WId winId() const { return winId_; }
  void setGeometry(const QRect&);
  Qt::WindowState setWindowState(Qt::WindowState state);
  void setOpacity(qreal level);
  void raise();
  void lower();

  EGLSurface eglSurface() const { return eglSurface_; }

 private:
  void moveResize(const QRect& rect);

  QHybrisScreen* screen_;
  SfSurface* sfSurface_;
  EGLSurface eglSurface_;
  Qt::WindowState state_;
  QRect geometry_;
  int layer_;
  WId winId_;
};

#endif  // QHYBRISWINDOW_H
