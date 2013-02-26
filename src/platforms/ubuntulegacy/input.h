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

#ifndef QUBUNTULEGACYINPUT_H
#define QUBUNTULEGACYINPUT_H

#include "base/input.h"
#include <cstring>  // input_stack_compatibility_layer.h needs this for size_t.
#include <input/input_stack_compatibility_layer.h>

class QUbuntuLegacyIntegration;

class QUbuntuLegacyInput : public QUbuntuBaseInput {
 public:
  QUbuntuLegacyInput(QUbuntuLegacyIntegration* integration);
  ~QUbuntuLegacyInput();

  QAtomicInt stopping_;

 private:
  InputStackConfiguration config_;
  AndroidEventListener listener_;
};

#endif  // QUBUNTULEGACYINPUT_H
