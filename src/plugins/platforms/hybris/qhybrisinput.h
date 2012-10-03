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

  QList<QWindowSystemInterface::TouchPoint> mTouchPoints;
  QTouchDevice* mTouchDevice;
  QHybrisIntegration* mIntegration;
  QAtomicInt mStopping;

 private:
  InputStackConfiguration mConfig;
  AndroidEventListener mListener;
};

#endif  // QHYBRISINPUT_H
