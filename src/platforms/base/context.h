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

#ifndef QUBUNTUBASECONTEXT_H
#define QUBUNTUBASECONTEXT_H

#include <qpa/qplatformopenglcontext.h>
#include "screen.h"

class QUbuntuBaseContext : public QPlatformOpenGLContext {
 public:
  QUbuntuBaseContext(QUbuntuBaseScreen* screen,
                     QUbuntuBaseContext* share);
  ~QUbuntuBaseContext();

  // QPlatformOpenGLContext methods.
  QSurfaceFormat format() const { return screen_->surfaceFormat(); }
  void swapBuffers(QPlatformSurface* surface);
  bool makeCurrent(QPlatformSurface* surface);
  void doneCurrent();
  bool isValid() const { return eglContext_ != EGL_NO_CONTEXT; }
  void (*getProcAddress(const QByteArray& procName)) ();
  EGLContext eglContext() const { return eglContext_; }

 private:
  QUbuntuBaseScreen* screen_;
  EGLContext eglContext_;
  EGLDisplay eglDisplay_;
};

#endif  //QUBUNTUBASECONTEXT_H
