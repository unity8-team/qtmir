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
#include <ubuntu/application/instance.h>
#include <ubuntu/application/ui/window.h>

#define IS_OPAQUE_FLAG 1

class QUbuntuScreen;
class QUbuntuInput;

class QUbuntuWindow : public QUbuntuBaseWindow {
 public:
  QUbuntuWindow(QWindow* w, QUbuntuScreen* screen, QUbuntuInput* input, bool systemSession, UApplicationInstance* instance, bool isShell);
  ~QUbuntuWindow();

  // QPlatformWindow methods.
  void setGeometry(const QRect&);
  void setWindowState(Qt::WindowState state);
  void setVisible(bool visible);

  QUbuntuInput* input_;

 private:
  void createWindow();
  void moveResize(const QRect& rect);

  UAUiWindow* window_;
  Qt::WindowState state_;
  QRect geometry_;
  bool systemSession_;
  UApplicationInstance* uainstance_;
  UAUiWindowProperties* wprops_;
  QUbuntuScreen* screen_;
  bool isShell_;
};

#endif  // QUBUNTUWINDOW_H
