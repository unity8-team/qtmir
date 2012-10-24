// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "window.h"
#include "screen.h"
#include "logging.h"
#include <qpa/qwindowsysteminterface.h>

QHybrisBaseWindow::QHybrisBaseWindow(QWindow* w, QHybrisBaseScreen* screen)
    : QPlatformWindow(w)
    , screen_(screen) {
  DASSERT(screen != NULL);
  static int id = 1;
  id_ = id++;
  DLOG("QHybrisBaseWindow::QHybrisBaseWindow (this=%p, screen=%p)", this, screen);
}

QHybrisBaseWindow::~QHybrisBaseWindow() {
  DLOG("QHybrisBaseWindow::~QHybrisBaseWindow");
  eglDestroySurface(screen_->eglDisplay(), eglSurface_);
}

void QHybrisBaseWindow::createSurface(EGLNativeWindowType nativeWindow) {
  DLOG("QHybrisBaseWindow::createSurface (this=%p, nativeWindow=%u)", this, nativeWindow);
  ASSERT((eglSurface_ = eglCreateWindowSurface(
      screen_->eglDisplay(), screen_->eglConfig(), nativeWindow, NULL)) != EGL_NO_SURFACE);
}
