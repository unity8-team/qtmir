// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTULEGACYWINDOW_H
#define QUBUNTULEGACYWINDOW_H

#include "base/window.h"

struct SfClient;
struct SfSurface;
class QUbuntuLegacyScreen;

class QUbuntuLegacyWindow : public QUbuntuBaseWindow {
 public:
  QUbuntuLegacyWindow(QWindow* w, QUbuntuLegacyScreen* screen);
  ~QUbuntuLegacyWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  void setWindowState(Qt::WindowState state);
  void setOpacity(qreal level);
  void raise();
  void lower();

 private:
  void moveResize(const QRect& rect);

  QUbuntuLegacyScreen* screen_;
  SfSurface* sfSurface_;
  Qt::WindowState state_;
  QRect geometry_;
  int layer_;
};

#endif  // QUBUNTULEGACYWINDOW_H
