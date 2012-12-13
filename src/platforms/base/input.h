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

  virtual void handleTouchEvent(QWindow* window, ulong timestamp, QTouchDevice* device,
                                 const QList<struct QWindowSystemInterface::TouchPoint> &points);
  virtual void handleKeyEvent(QWindow* window, ulong timestamp, QEvent::Type type, int key,
                              Qt::KeyboardModifiers modifiers, const QString& text);

  void postEvent(QWindow* window, const Event* event);
  QHybrisBaseIntegration* integration() const { return integration_; }

 private:
  void dispatchMotionEvent(QWindow* window, const Event* event);
  void dispatchKeyEvent(QWindow* window, const Event* event);
  void dispatchHWSwitchEvent(QWindow* window, const Event* event);

  QHybrisBaseIntegration* integration_;
  QTouchDevice* touchDevice_;
  QList<QWindowSystemInterface::TouchPoint> touchPoints_;
  const QByteArray eventFilterType_;
  const QEvent::Type eventType_;
};

#endif  // QHYBRISBASEINPUT_H
