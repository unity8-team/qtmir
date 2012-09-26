// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisintegration.h"
#include "qhybriswindow.h"
#include "qhybrisbackingstore.h"
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QtGui/QScreen>
#include <EGL/egl.h>

QT_BEGIN_NAMESPACE

QHybrisIntegration::QHybrisIntegration()
    : mFontDb(new QGenericUnixFontDatabase())
    , mScreen(new QHybrisScreen(EGL_DEFAULT_DISPLAY)) {
  screenAdded(mScreen);
#ifdef QHYBRIS_DEBUG
  qWarning("QHybrisIntegration\n");
#endif
}

QHybrisIntegration::~QHybrisIntegration() {
  delete mScreen;
}

bool QHybrisIntegration::hasCapability(QPlatformIntegration::Capability cap) const {
  switch (cap) {
    case ThreadedPixmaps: return true;
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    default: return QPlatformIntegration::hasCapability(cap);
  }
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) const {
#ifdef QHYBRIS_DEBUG
  qWarning("QHybrisIntegration::createPlatformWindow %p\n",window);
#endif
  QPlatformWindow* w = new QHybrisWindow(window);
  w->requestActivateWindow();
  return w;
}

QPlatformBackingStore* QHybrisIntegration::createPlatformBackingStore(QWindow* window) const {
#ifdef QHYBRIS_DEBUG
  qWarning("QHybrisIntegration::createWindowSurface %p\n", window);
#endif
  return new QHybrisBackingStore(window);
}

QPlatformOpenGLContext* QHybrisIntegration::createPlatformOpenGLContext(
    QOpenGLContext *context) const {
  return static_cast<QHybrisScreen*>(context->screen()->handle())->platformContext();
}

QPlatformFontDatabase* QHybrisIntegration::fontDatabase() const {
  return mFontDb;
}

QAbstractEventDispatcher* QHybrisIntegration::guiThreadEventDispatcher() const {
  return createUnixEventDispatcher();
}

QVariant QHybrisIntegration::styleHint(QPlatformIntegration::StyleHint hint) const {
  if (hint == QPlatformIntegration::ShowIsFullScreen)
    return true;
  return QPlatformIntegration::styleHint(hint);
}

QT_END_NAMESPACE
