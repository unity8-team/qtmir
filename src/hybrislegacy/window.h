// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISWINDOW_H
#define QHYBRISWINDOW_H

#include "base/window.h"

struct SfClient;
struct SfSurface;
class QHybrisScreen;

class QHybrisWindow : public QHybrisBaseWindow {
 public:
  QHybrisWindow(QWindow* w, QHybrisScreen* screen);
  ~QHybrisWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  Qt::WindowState setWindowState(Qt::WindowState state);
  void setOpacity(qreal level);
  void raise();
  void lower();

 private:
  void moveResize(const QRect& rect);

  QHybrisScreen* screen_;
  SfSurface* sfSurface_;
  Qt::WindowState state_;
  QRect geometry_;
  int layer_;
};

#endif  // QHYBRISWINDOW_H
