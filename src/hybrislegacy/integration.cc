// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "integration.h"
#include "window.h"
#include "input.h"
#include "base/logging.h"
#include <QTimer>

// That value seems to work on every systems and applications tested so far.
static const int kInputDelay = 1000;

QHybrisIntegration::QHybrisIntegration()
    : window_(NULL)
    , screen_(new QHybrisScreen())
    , input_(NULL) {
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
}

void QHybrisIntegration::initInput() {
  DLOG("QHybrisIntegration::initInput (this=%p)", this);
  input_ = new QHybrisInput(this);
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QHybrisIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QHybrisIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QHybrisIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QHybrisIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  ASSERT(window_ == NULL);  // FIXME(loicm) Multiple windows are not supported yet.
  window_ = new QHybrisWindow(window, static_cast<QHybrisScreen*>(screen_));
  window_->requestActivateWindow();
  return window_;
}
