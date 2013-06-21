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

#ifndef QUBUNTUINPUT_H
#define QUBUNTUINPUT_H

#include "base/input.h"

class QUbuntuIntegration;

class QUbuntuInput : public QUbuntuBaseInput {
 public:
  QUbuntuInput(QUbuntuIntegration* integration);
  ~QUbuntuInput();

  // QUbuntuBaseInput methods.
  void handleTouchEvent(QWindow* window, ulong timestamp, QTouchDevice* device,
                        const QList<struct QWindowSystemInterface::TouchPoint> &points);

  void setSessionType(uint sessionType);

 private:
  uint sessionType_;
};

#endif  // QUBUNTUINPUT_H
