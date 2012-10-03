// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybriswindow.h"
#include "qhybrislogging.h"
#include <qpa/qwindowsysteminterface.h>

QHybrisWindow::QHybrisWindow(QWindow* w)
    : QPlatformWindow(w) {
  static int serialNo = 0;
  m_winId = ++serialNo;
  QRect screenGeometry(screen()->availableGeometry());
  if (w->geometry() != screenGeometry)
    QWindowSystemInterface::handleGeometryChange(w, screenGeometry);
  DLOG("created QHybrisWindow (this=%p)", this);
}

void QHybrisWindow::setGeometry(const QRect&) {
  // We only support full-screen windows.
  QRect rect(screen()->availableGeometry());
  QWindowSystemInterface::handleGeometryChange(window(), rect);
  QPlatformWindow::setGeometry(rect);
  DLOG("deleted QHybrisWindow");
}

WId QHybrisWindow::winId() const {
  DLOG("QHybrisWindow::winId");
  return m_winId;
}
