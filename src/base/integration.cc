// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "integration.h"
#include "native_interface.h"
#include "backing_store.h"
#include "screen.h"
#include "context.h"
#include "logging.h"
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QOpenGLContext>

extern "C" void init_hybris();

QHybrisBaseIntegration::QHybrisBaseIntegration()
    : eventDispatcher_(createUnixEventDispatcher())
    , nativeInterface_(new QHybrisBaseNativeInterface())
    , fontDb_(new QGenericUnixFontDatabase())
    , context_(NULL) {
  static bool once = false;
  if (!once) {
    // Init libhybris ensuring the libs are loaded and threading is all setup.
    DLOG("initializing libhybris");
    init_hybris();
    once = true;
  }
  QGuiApplicationPrivate::instance()->setEventDispatcher(eventDispatcher_);
  DLOG("QHybrisBaseIntegration::QHybrisBaseIntegration (this=%p)", this);
}

QHybrisBaseIntegration::~QHybrisBaseIntegration() {
  DLOG("QHybrisBaseIntegration::~QHybrisBaseIntegration");
  delete fontDb_;
  delete nativeInterface_;
}

bool QHybrisBaseIntegration::hasCapability(QPlatformIntegration::Capability cap) const {
  DLOG("QHybrisBaseIntegration::hasCapability (this=%p)", this);
  switch (cap) {
    case ThreadedPixmaps: {
      return true;
    } case OpenGL: {
      return true;
    }
    case ThreadedOpenGL: {
      qEnvironmentVariableIsEmpty("QTHYBRIS_NO_THREADED_OPENGL") ? return true : return false;
    }
    default: {
      return QPlatformIntegration::hasCapability(cap);
    }
  }
}

QPlatformBackingStore* QHybrisBaseIntegration::createPlatformBackingStore(QWindow* window) const {
  DLOG("QHybrisBaseIntegration::createPlatformBackingStore (this=%p, window=%p)", this, window);
  return new QHybrisBaseBackingStore(window);
}

QPlatformOpenGLContext* QHybrisBaseIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context) const {
  DLOG("QHybrisBaseIntegration::createPlatformOpenGLContext const (this=%p, context=%p)", this,
       context);
  return const_cast<QHybrisBaseIntegration*>(this)->createPlatformOpenGLContext(context);
}

QPlatformOpenGLContext* QHybrisBaseIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context) {
  DLOG("QHybrisBaseIntegration::createPlatformOpenGLContext (this=%p, context=%p)", this, context);
  if (!context_)
    context_ = new QHybrisBaseContext(static_cast<QHybrisBaseScreen*>(context->screen()->handle()));
  return context_;
}
