// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISINPUT_H
#define QHYBRISINPUT_H

#include <cstring>  // input_stack_compatibility_layer.h needs this for size_t.
#include <input/input_stack_compatibility_layer.h>
#include <qpa/qwindowsysteminterface.h>

class QHybrisIntegration;

class QHybrisInput {
 public:
  QHybrisInput(QHybrisIntegration* integration);
  ~QHybrisInput();

  QList<QWindowSystemInterface::TouchPoint> touchPoints_;
  QTouchDevice* touchDevice_;
  QHybrisIntegration* integration_;
  QAtomicInt stopping_;
  const QByteArray eventFilterType_;

 private:
  InputStackConfiguration config_;
  AndroidEventListener listener_;
};

#endif  // QHYBRISINPUT_H
