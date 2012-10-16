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
#include <QTimer>
#include <EGL/egl.h>

extern "C" void init_hybris();

// That value seems to work on every systems and applications tested so far.
static const int kInputDelay = 1000;

QHybrisIntegration::QHybrisIntegration()
    : eventDispatcher_(createUnixEventDispatcher())
    , window_(NULL)
    , fontDb_(new QGenericUnixFontDatabase())
    , screen_(new QHybrisScreen())
    , input_(NULL) {
  // Init libhybris ensuring the libs are loaded and threading is all setup.
  static bool once = false;
  if (!once) {
    DLOG("initializing libhybris");
    init_hybris();
    once = true;
  }

  QGuiApplicationPrivate::instance()->setEventDispatcher(eventDispatcher_);
  screenAdded(screen_);

  if (qEnvironmentVariableIsEmpty("QTHYBRIS_NO_INPUT")) {
    // Input initialization needs to be delayed in order to avoid crashes in the input stack.
    int delay = kInputDelay;
    QByteArray swapIntervalString = qgetenv("QTHYBRIS_INPUT_DELAY");
    if (!swapIntervalString.isEmpty()) {
      bool valid;
      delay = qMax(1, swapIntervalString.toInt(&valid));
      if (!valid)
        delay = kInputDelay;
    }
    DLOG("delaying input initialization for %d ms", delay);
    QTimer::singleShot(delay, this, SLOT(initInput()));
  }

  DLOG("QHybrisIntegration::QHybrisIntegration (this=%p)", this);
}

QHybrisIntegration::~QHybrisIntegration() {
  DLOG("QHybrisIntegration::~QHybrisIntegration");
  delete input_;
  delete screen_;
  delete fontDb_;
}

void QHybrisIntegration::initInput() {
  DLOG("QHybrisIntegration::initInput (this=%p)", this);
  input_ = new QHybrisInput(this);
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
