// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINPUT_H
#define QHYBRISINPUT_H

#include "base/input.h"
#include <cstring>  // input_stack_compatibility_layer.h needs this for size_t.
#include <input/input_stack_compatibility_layer.h>

class QHybrisIntegration;

class QHybrisInput : public QHybrisBaseInput {
 public:
  QHybrisInput(QHybrisIntegration* integration);
  ~QHybrisInput();

  QAtomicInt stopping_;

 private:
  InputStackConfiguration config_;
  AndroidEventListener listener_;
};

#endif  // QHYBRISINPUT_H
