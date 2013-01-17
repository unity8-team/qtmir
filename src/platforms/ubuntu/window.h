// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTUWINDOW_H
#define QUBUNTUWINDOW_H

#include "base/window.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

class QUbuntuScreen;
class QUbuntuInput;

class QUbuntuWindow : public QUbuntuBaseWindow {
 public:
  QUbuntuWindow(QWindow* w, QUbuntuScreen* screen, QUbuntuInput* input, bool systemSession);
  ~QUbuntuWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  Qt::WindowState setWindowState(Qt::WindowState state);
  void setVisible(bool visible);

  QUbuntuInput* input_;

 private:
  void createWindow();
  void moveResize(const QRect& rect);

  ubuntu_application_ui_surface surface_;
  Qt::WindowState state_;
  QRect geometry_;
  bool systemSession_;
};

#endif  // QUBUNTUWINDOW_H
