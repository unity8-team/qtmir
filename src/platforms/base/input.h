// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QUBUNTUBASEINPUT_H
#define QUBUNTUBASEINPUT_H

#include <qpa/qwindowsysteminterface.h>

class QUbuntuBaseIntegration;

class QUbuntuBaseInput : public QObject {
  Q_OBJECT

 public:
  QUbuntuBaseInput(QUbuntuBaseIntegration* integration, int maxPointCount);
  ~QUbuntuBaseInput();

  // QObject methods.
  void customEvent(QEvent* event);

  virtual void handleTouchEvent(QWindow* window, ulong timestamp, QTouchDevice* device,
                                 const QList<struct QWindowSystemInterface::TouchPoint> &points);
  virtual void handleKeyEvent(QWindow* window, ulong timestamp, QEvent::Type type, int key,
                              Qt::KeyboardModifiers modifiers, const QString& text);

  void postEvent(QWindow* window, const void* event);
  QUbuntuBaseIntegration* integration() const { return integration_; }

 protected:
  virtual void dispatchKeyEvent(QWindow* window, const void* event);
  void dispatchMotionEvent(QWindow* window, const void* event);
  void dispatchHWSwitchEvent(QWindow* window, const void* event);

 private:
  QUbuntuBaseIntegration* integration_;
  QTouchDevice* touchDevice_;
  QList<QWindowSystemInterface::TouchPoint> touchPoints_;
  const QByteArray eventFilterType_;
  const QEvent::Type eventType_;
};

#endif  // QUBUNTUBASEINPUT_H
