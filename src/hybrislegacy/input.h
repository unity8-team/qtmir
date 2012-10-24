// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISLEGACYINPUT_H
#define QHYBRISLEGACYINPUT_H

#include "base/input.h"
#include <cstring>  // input_stack_compatibility_layer.h needs this for size_t.
#include <input/input_stack_compatibility_layer.h>

class QHybrisLegacyIntegration;

class QHybrisLegacyInput : public QHybrisBaseInput {
 public:
  QHybrisLegacyInput(QHybrisLegacyIntegration* integration);
  ~QHybrisLegacyInput();

  QAtomicInt stopping_;

 private:
  InputStackConfiguration config_;
  AndroidEventListener listener_;
};

#endif  // QHYBRISLEGACYINPUT_H
