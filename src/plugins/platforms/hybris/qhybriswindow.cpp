// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybriswindow.h"
#include "qhybrislogging.h"
#include <qpa/qwindowsysteminterface.h>

QHybrisWindow::QHybrisWindow(QWindow* w)
    : QPlatformWindow(w) {
  static int serialNo = 0;
  winId_ = ++serialNo;
  QRect screenGeometry(screen()->availableGeometry());
  if (w->geometry() != screenGeometry)
    QWindowSystemInterface::handleGeometryChange(w, screenGeometry);
  DLOG("QHybrisWindow::QHybrisWindow (this=%p)", this);
}

QHybrisWindow::~QHybrisWindow() {
  DLOG("QHybrisWindow::~QHybrisWindow");
}

void QHybrisWindow::setGeometry(const QRect& rect) {
  Q_UNUSED(rect);
  DLOG("QHybrisWindow::setGeometry (this=%p)", this);
  // We only support fullscreen windows.
  QRect geometry(screen()->availableGeometry());
  QWindowSystemInterface::handleGeometryChange(window(), geometry);
  QPlatformWindow::setGeometry(geometry);
}
