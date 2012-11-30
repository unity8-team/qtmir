// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISBASEINPUT_H
#define QHYBRISBASEINPUT_H

#include <qpa/qwindowsysteminterface.h>

class QHybrisBaseIntegration;
struct Event;

class QHybrisBaseInput : public QObject {
  Q_OBJECT

 public:
  QHybrisBaseInput(QHybrisBaseIntegration* integration, int maxPointCount);
  ~QHybrisBaseInput();

  // QObject methods.
  void customEvent(QEvent* event);

  void postEvent(QWindow* window, const Event* event);
  QHybrisBaseIntegration* integration() const { return integration_; }

 private:
  void handleMotionEvent(QWindow* window, const Event* event);
  void handleKeyEvent(QWindow* window, const Event* event);
  void handleHWSwitchEvent(QWindow* window, const Event* event);

  QHybrisBaseIntegration* integration_;
  QTouchDevice* touchDevice_;
  QList<QWindowSystemInterface::TouchPoint> touchPoints_;
  const QByteArray eventFilterType_;
  const QEvent::Type eventType_;
};

#endif  // QHYBRISBASEINPUT_H
