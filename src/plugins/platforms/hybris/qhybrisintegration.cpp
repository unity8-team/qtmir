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
    : mEventDispatcher(createUnixEventDispatcher())
    , mWindow(NULL)
    , mFontDb(new QGenericUnixFontDatabase())
    , mScreen(new QHybrisScreen())
    , mInput(new QHybrisInput(this)) {
  QGuiApplicationPrivate::instance()->setEventDispatcher(mEventDispatcher);
  screenAdded(mScreen);
  DLOG("created QHybrisIntegration (this=%p)", this);
}

QHybrisIntegration::~QHybrisIntegration() {
  delete mInput;
  delete mScreen;
  delete mFontDb;
  DLOG("deleted QHybrisIntegration");
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

QAbstractEventDispatcher *QHybrisIntegration::guiThreadEventDispatcher() const {
  DLOG("QHybrisIntegration::guiThreadEventDispatcher (this=%p)", this);
  return mEventDispatcher;
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QHybrisIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QHybrisIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QHybrisIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  ASSERT(mWindow == NULL);  // FIXME(loicm) Multiple windows are not supported yet.
  mWindow = new QHybrisWindow(window);
  mWindow->requestActivateWindow();
  return mWindow;
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

QPlatformFontDatabase* QHybrisIntegration::fontDatabase() const {
  DLOG("QHybrisIntegration::fontDatabase (this=%p)", this);
  return mFontDb;
}

QVariant QHybrisIntegration::styleHint(QPlatformIntegration::StyleHint hint) const {
  DLOG("QHybrisIntegration::stylehint (this=%p)", this);
  if (hint == QPlatformIntegration::ShowIsFullScreen)
    return true;
  return QPlatformIntegration::styleHint(hint);
}
