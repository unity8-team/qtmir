// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

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
