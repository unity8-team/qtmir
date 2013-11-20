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

#ifndef QUBUNTUBASEINTEGRATION_H
#define QUBUNTUBASEINTEGRATION_H

#include <qpa/qplatformintegration.h>

class QAbstractEventDispatcher;

class QUbuntuBaseIntegration : public QPlatformIntegration {
 public:
  QUbuntuBaseIntegration();
  ~QUbuntuBaseIntegration();

  // QPlatformIntegration methods.
  bool hasCapability(QPlatformIntegration::Capability cap) const;
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  QAbstractEventDispatcher* guiThreadEventDispatcher() const { return eventDispatcher_; }
#else
  QAbstractEventDispatcher *createEventDispatcher() const;
#endif
  QPlatformNativeInterface* nativeInterface() const { return nativeInterface_; }
  QPlatformServices *services() const { return platformServices_; }
  QPlatformBackingStore* createPlatformBackingStore(QWindow* window) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context) const;
  QPlatformOpenGLContext* createPlatformOpenGLContext(QOpenGLContext* context);
  QPlatformFontDatabase* fontDatabase() const { return fontDb_; }
  QStringList themeNames() const;
  QPlatformTheme* createPlatformTheme(const QString& name) const;

 private:
  QPlatformNativeInterface* nativeInterface_;
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  QAbstractEventDispatcher* eventDispatcher_;
#endif
  QPlatformFontDatabase* fontDb_;
  QPlatformServices* platformServices_;
};

#endif  // QUBUNTUBASEINTEGRATION_H
