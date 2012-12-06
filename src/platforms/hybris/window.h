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
  void setVisible(bool visible);

  QHybrisInput* input_;

 private:
  void createWindow();
  void moveResize(const QRect& rect);

  ubuntu_application_ui_surface surface_;
  Qt::WindowState state_;
  QRect geometry_;
  bool windowCreated_;
};

#endif  // QHYBRISWINDOW_H
