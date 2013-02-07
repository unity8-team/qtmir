// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QUBUNTUSCREEN_H
#define QUBUNTUSCREEN_H

#include "base/screen.h"

class QUbuntuScreen : public QUbuntuBaseScreen {
 public:
  QUbuntuScreen();
  ~QUbuntuScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }
  QRect availableGeometry() const { return availableGeometry_; }

  Qt::ScreenOrientation nativeOrientation() const { return nativeOrientation_; }

 private:
  QRect geometry_;
  QRect availableGeometry_;

  Qt::ScreenOrientation nativeOrientation_;
};

#endif  // QUBUNTUSCREEN_H
