// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "integration.h"
#include "native_interface.h"
#include "backing_store.h"
#include "screen.h"
#include "context.h"
#include "clipboard.h"
#include "logging.h"
#include "theme.h"
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QOpenGLContext>

extern "C" void init_hybris();

QUbuntuBaseIntegration::QUbuntuBaseIntegration()
    : eventDispatcher_(createUnixEventDispatcher())
    , nativeInterface_(new QUbuntuBaseNativeInterface())
    , fontDb_(new QGenericUnixFontDatabase())
    , clipboard_(new QUbuntuBaseClipboard()) {
  static bool once = false;
  if (!once) {
    // Init libhybris ensuring the libs are loaded and threading is all setup.
    DLOG("initializing libhybris");
    init_hybris();
    once = true;
  }
  QGuiApplicationPrivate::instance()->setEventDispatcher(eventDispatcher_);
  DLOG("QUbuntuBaseIntegration::QUbuntuBaseIntegration (this=%p)", this);
}

QUbuntuBaseIntegration::~QUbuntuBaseIntegration() {
  DLOG("QUbuntuBaseIntegration::~QUbuntuBaseIntegration");
  delete clipboard_;
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
