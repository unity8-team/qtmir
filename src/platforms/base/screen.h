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

#ifndef QUBUNTUBASESCREEN_H
#define QUBUNTUBASESCREEN_H

#include <qpa/qplatformscreen.h>
#include <QSurfaceFormat>
#include <EGL/egl.h>

class QUbuntuBaseScreen : public QPlatformScreen {
 public:
  QUbuntuBaseScreen();
  ~QUbuntuBaseScreen();

  // QPlatformScreen methods.
  QImage::Format format() const { return format_; }
  int depth() const { return depth_; }

  // New methods.
  QSurfaceFormat surfaceFormat() const { return surfaceFormat_; }
  EGLDisplay eglDisplay() const { return eglDisplay_; }
  EGLConfig eglConfig() const { return eglConfig_; }

 private:
  QImage::Format format_;
  int depth_;
  QSurfaceFormat surfaceFormat_;
  EGLDisplay eglDisplay_;
  EGLConfig eglConfig_;
};

#endif  // QUBUNTUBASESCREEN_H
