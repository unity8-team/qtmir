// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINPUT_H
#define QHYBRISINPUT_H

#include "base/input.h"

class QHybrisIntegration;

class QHybrisInput : public QHybrisBaseInput {
 public:
  QHybrisInput(QHybrisIntegration* integration);
  ~QHybrisInput();
};

#endif  // QHYBRISINPUT_H
