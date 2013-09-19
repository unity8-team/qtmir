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

#include "integration.h"
#include "native_interface.h"
#include "backing_store.h"
#include "screen.h"
#include "context.h"
#include "logging.h"
#include "theme.h"
#include "platformservices.h"
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QOpenGLContext>

QUbuntuBaseIntegration::QUbuntuBaseIntegration()
    : eventDispatcher_(createUnixEventDispatcher())
    , nativeInterface_(new QUbuntuBaseNativeInterface())
    , fontDb_(new QGenericUnixFontDatabase())
    , platformServices_(new QUbuntuBasePlatformServices()) {
  QGuiApplicationPrivate::instance()->setEventDispatcher(eventDispatcher_);
  DLOG("QUbuntuBaseIntegration::QUbuntuBaseIntegration (this=%p)", this);
}

QUbuntuBaseIntegration::~QUbuntuBaseIntegration() {
  DLOG("QUbuntuBaseIntegration::~QUbuntuBaseIntegration");
  delete fontDb_;
  delete nativeInterface_;
}

bool QUbuntuBaseIntegration::hasCapability(QPlatformIntegration::Capability cap) const {
  DLOG("QUbuntuBaseIntegration::hasCapability (this=%p)", this);
  switch (cap) {
    case ThreadedPixmaps: {
      return true;
    } case OpenGL: {
      return true;
    }
    case ThreadedOpenGL: {
      if (qEnvironmentVariableIsEmpty("QTUBUNTU_NO_THREADED_OPENGL")) {
        return true;
      } else {
        DLOG("disabled threaded OpenGL");
        return false;
      }
    }
    default: {
      return QPlatformIntegration::hasCapability(cap);
    }
  }
}

QPlatformBackingStore* QUbuntuBaseIntegration::createPlatformBackingStore(QWindow* window) const {
  DLOG("QUbuntuBaseIntegration::createPlatformBackingStore (this=%p, window=%p)", this, window);
  return new QUbuntuBaseBackingStore(window);
}

QPlatformOpenGLContext* QUbuntuBaseIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context) const {
  DLOG("QUbuntuBaseIntegration::createPlatformOpenGLContext const (this=%p, context=%p)", this,
       context);
  return const_cast<QUbuntuBaseIntegration*>(this)->createPlatformOpenGLContext(context);
}

QPlatformOpenGLContext* QUbuntuBaseIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context) {
  DLOG("QUbuntuBaseIntegration::createPlatformOpenGLContext (this=%p, context=%p)", this, context);
  return new QUbuntuBaseContext(static_cast<QUbuntuBaseScreen*>(context->screen()->handle()));
}

QStringList QUbuntuBaseIntegration::themeNames() const {
  DLOG("QUbuntuBaseIntegration::themeNames (this=%p)", this);
  return QStringList(QUbuntuTheme::name);
}

QPlatformTheme* QUbuntuBaseIntegration::createPlatformTheme(const QString& name) const {
  Q_UNUSED(name);
  DLOG("QUbuntuBaseIntegration::createPlatformTheme (this=%p)", this);
  return new QUbuntuTheme();
}
