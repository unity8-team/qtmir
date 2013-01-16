// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISLEGACYWINDOW_H
#define QHYBRISLEGACYWINDOW_H

#include "base/window.h"

struct SfClient;
struct SfSurface;
class QHybrisLegacyScreen;

class QHybrisLegacyWindow : public QHybrisBaseWindow {
 public:
  QHybrisLegacyWindow(QWindow* w, QHybrisLegacyScreen* screen);
  ~QHybrisLegacyWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  void setWindowState(Qt::WindowState state);
  void setOpacity(qreal level);
  void raise();
  void lower();

 private:
  void moveResize(const QRect& rect);

  QHybrisLegacyScreen* screen_;
  SfSurface* sfSurface_;
  Qt::WindowState state_;
  QRect geometry_;
  int layer_;
};

#endif  // QHYBRISLEGACYWINDOW_H
