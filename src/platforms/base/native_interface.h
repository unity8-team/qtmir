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

#ifndef QUBUNTUBASENATIVEINTERFACE_H
#define QUBUNTUBASENATIVEINTERFACE_H

#include <qpa/qplatformnativeinterface.h>

class QUbuntuBaseNativeInterface : public QPlatformNativeInterface {
 public:
  enum ResourceType { EglDisplay, EglContext, NativeOrientation, Display };

  QUbuntuBaseNativeInterface();
  ~QUbuntuBaseNativeInterface();

  // QPlatformNativeInterface methods.
  void* nativeResourceForContext(const QByteArray& resourceString, QOpenGLContext* context);
  void* nativeResourceForScreen(const QByteArray& resourceString, QScreen* screen);
  void* nativeResourceForWindow(const QByteArray& resourceString, QWindow* window);

  // New methods.
  const QByteArray& genericEventFilterType() const { return genericEventFilterType_; }

 private:
  const QByteArray genericEventFilterType_;
  Qt::ScreenOrientation* nativeOrientation_;
};

#endif  // QUBUNTUNATIVEINTERFACE_H
