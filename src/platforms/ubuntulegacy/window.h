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
