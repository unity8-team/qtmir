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

  // QHybrisBaseInput methods.
  void handleTouchEvent(QWindow* window, ulong timestamp, QTouchDevice* device,
                        const QList<struct QWindowSystemInterface::TouchPoint> &points);

  void setSessionType(uint sessionType);

 private:
  uint sessionType_;
};

#endif  // QHYBRISINPUT_H
