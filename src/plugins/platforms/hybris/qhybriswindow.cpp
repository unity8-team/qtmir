// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybriswindow.h"
#include <qpa/qwindowsysteminterface.h>

QT_BEGIN_NAMESPACE

QHybrisWindow::QHybrisWindow(QWindow* w)
    : QPlatformWindow(w) {
  static int serialNo = 0;
  m_winid  = ++serialNo;
#ifdef QHYBRIS_DEBUG
  qWarning("QEglWindow %p: %p 0x%x\n", this, w, uint(m_winid));
#endif

  QRect screenGeometry(screen()->availableGeometry());
  if (w->geometry() != screenGeometry) {
    QWindowSystemInterface::handleGeometryChange(w, screenGeometry);
  }
}

void QHybrisWindow::setGeometry(const QRect&) {
  // We only support full-screen windows.
  QRect rect(screen()->availableGeometry());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
}

WId QHybrisWindow::winId() const {
  return m_winid;
}

QT_END_NAMESPACE
