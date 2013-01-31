// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "window.h"
#include "screen.h"
#include "logging.h"
#include <qpa/qwindowsysteminterface.h>

QUbuntuBaseWindow::QUbuntuBaseWindow(QWindow* w, QUbuntuBaseScreen* screen)
    : QPlatformWindow(w)
    , screen_(screen) {
  DASSERT(screen != NULL);
  static int id = 1;
  id_ = id++;
  DLOG("QUbuntuBaseWindow::QUbuntuBaseWindow (this=%p, screen=%p)", this, screen);
}

QUbuntuBaseWindow::~QUbuntuBaseWindow() {
  DLOG("QUbuntuBaseWindow::~QUbuntuBaseWindow");
  eglDestroySurface(screen_->eglDisplay(), eglSurface_);
}

void QUbuntuBaseWindow::createSurface(EGLNativeWindowType nativeWindow) {
  DLOG("QUbuntuBaseWindow::createSurface (this=%p, nativeWindow=%u)", this, nativeWindow);
  ASSERT((eglSurface_ = eglCreateWindowSurface(
      screen_->eglDisplay(), screen_->eglConfig(), nativeWindow, NULL)) != EGL_NO_SURFACE);
}
