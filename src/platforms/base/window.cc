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

#include "window.h"
#include "screen.h"
#include "logging.h"
#include <qpa/qwindowsysteminterface.h>

QUbuntuBaseWindow::QUbuntuBaseWindow(QWindow* w, QUbuntuBaseScreen* screen)
    : QPlatformWindow(w)
    , screen_(screen)
    , eglSurface_(EGL_NO_SURFACE) {
  DASSERT(screen != NULL);
  static int id = 1;
  id_ = id++;
  DLOG("QUbuntuBaseWindow::QUbuntuBaseWindow (this=%p, screen=%p)", this, screen);
}

QUbuntuBaseWindow::~QUbuntuBaseWindow() {
  DLOG("QUbuntuBaseWindow::~QUbuntuBaseWindow");
}

void QUbuntuBaseWindow::createEGLSurface(EGLNativeWindowType nativeWindow) {
  DLOG("QUbuntuBaseWindow::createEGLSurface (this=%p, nativeWindow=%p)", this, reinterpret_cast<void*>(nativeWindow));
  ASSERT((eglSurface_ = eglCreateWindowSurface(
      screen_->eglDisplay(), screen_->eglConfig(), nativeWindow, NULL)) != EGL_NO_SURFACE);
}

void QUbuntuBaseWindow::destroyEGLSurface() {
  DLOG("QUbuntuBaseWindow::destroyEGLSurface (this=%p)", this);
  if (eglSurface_ != EGL_NO_SURFACE) {
    eglDestroySurface(screen_->eglDisplay(), eglSurface_);
    eglSurface_ = EGL_NO_SURFACE;
  }
}
