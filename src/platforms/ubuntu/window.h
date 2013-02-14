// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  void setWindowState(Qt::WindowState state);
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
