// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

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
