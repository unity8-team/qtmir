// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISWINDOW_H
#define QHYBRISWINDOW_H

#include "base/window.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

class QHybrisScreen;
class QHybrisInput;

class QHybrisWindow : public QHybrisBaseWindow {
 public:
  QHybrisWindow(QWindow* w, QHybrisScreen* screen, QHybrisInput* input);
  ~QHybrisWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  Qt::WindowState setWindowState(Qt::WindowState state);

  QHybrisInput* input_;

 private:
  Qt::WindowState setState(Qt::WindowState state);
  void moveResize(const QRect& rect);

  QHybrisScreen* screen_;
  ubuntu_application_ui_surface surface_;
  Qt::WindowState state_;
  QRect geometry_;
};

#endif  // QHYBRISWINDOW_H
