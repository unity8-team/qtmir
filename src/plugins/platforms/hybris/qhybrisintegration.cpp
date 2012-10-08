// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisintegration.h"
#include "qhybriswindow.h"
#include "qhybrisbackingstore.h"
#include "qhybrisinput.h"
#include "qhybrislogging.h"
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QtGui/QScreen>
#include <EGL/egl.h>

QHybrisIntegration::QHybrisIntegration()
    : eventDispatcher_(createUnixEventDispatcher())
    , window_(NULL)
    , fontDb_(new QGenericUnixFontDatabase())
    , screen_(new QHybrisScreen())
    , input_(NULL) {
  QGuiApplicationPrivate::instance()->setEventDispatcher(eventDispatcher_);
  screenAdded(screen_);
  DLOG("QHybrisIntegration::QHybrisIntegration (this=%p)", this);
}

QHybrisIntegration::~QHybrisIntegration() {
  DLOG("QHybrisIntegration::~QHybrisIntegration");
  delete input_;
  delete screen_;
  delete fontDb_;
}

bool QHybrisIntegration::hasCapability(QPlatformIntegration::Capability cap) const {
  DLOG("QHybrisIntegration::hasCapability (this=%p)", this);
  switch (cap) {
    case ThreadedPixmaps: {
      return true;
    }
    case OpenGL: {
      return true;
    }
    case ThreadedOpenGL: {
      return true;
    }
    default: {
      return QPlatformIntegration::hasCapability(cap);
    }
  }
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QHybrisIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QHybrisIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QHybrisIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  ASSERT(window_ == NULL);  // FIXME(loicm) Multiple windows are not supported yet.
  window_ = new QHybrisWindow(window);
  window_->requestActivateWindow();

  // FIXME(loicm) The deadlock still happens sometimes :/
  // Input initialization is delayed after the creation of the first window in order to avoid a
  // deadlock in the input stack.
  if (input_ == NULL)
    input_ = new QHybrisInput(this);

  return window_;
}

QPlatformBackingStore* QHybrisIntegration::createPlatformBackingStore(QWindow* window) const {
  DLOG("QHybrisIntegration::createPlatformBackingStore (this=%p, window=%p)", this, window);
  return new QHybrisBackingStore(window);
}

QPlatformOpenGLContext* QHybrisIntegration::createPlatformOpenGLContext(
    QOpenGLContext* context) const {
  DLOG("QHybrisIntegration::createPlatformOpenGLContext (this=%p, context=%p)", this, context);
  return static_cast<QHybrisScreen*>(context->screen()->handle())->platformContext();
}

QVariant QHybrisIntegration::styleHint(QPlatformIntegration::StyleHint hint) const {
  DLOG("QHybrisIntegration::stylehint (this=%p)", this);
  if (hint == QPlatformIntegration::ShowIsFullScreen)
    return true;
  return QPlatformIntegration::styleHint(hint);
}
