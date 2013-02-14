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

#ifndef QUBUNTULEGACYSCREEN_H
#define QUBUNTULEGACYSCREEN_H

#include "base/screen.h"

struct SfClient;

class QUbuntuLegacyScreen : public QUbuntuBaseScreen {
 public:
  QUbuntuLegacyScreen();
  ~QUbuntuLegacyScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }

  // New methods.
  SfClient* sfClient() const { return sfClient_; }

 private:
  QRect geometry_;
  SfClient* sfClient_;
};

#endif  // QUBUNTULEGACYSCREEN_H
