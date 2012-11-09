// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "integration.h"
#include "window.h"
#include "input.h"
#include "base/logging.h"
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>
#include <QTimer>

// That value seems to work on every systems and applications tested so far.
static const int kInputDelay = 1000;

QHybrisLegacyIntegration::QHybrisLegacyIntegration()
    : window_(NULL)
    , screen_(new QHybrisLegacyScreen())
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
  DLOG("QHybrisLegacyIntegration::QHybrisLegacyIntegration (this=%p)", this);
}

QHybrisLegacyIntegration::~QHybrisLegacyIntegration() {
  DLOG("QHybrisLegacyIntegration::~QHybrisLegacyIntegration");
  delete input_;
  delete inputContext_;
  delete screen_;
}

void QHybrisLegacyIntegration::initInput() {
  DLOG("QHybrisLegacyIntegration::initInput (this=%p)", this);
  input_ = new QHybrisLegacyInput(this);
  inputContext_ = QPlatformInputContextFactory::create();
}

QPlatformWindow* QHybrisLegacyIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QHybrisLegacyIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QHybrisLegacyIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QHybrisLegacyIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QHybrisLegacyIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  ASSERT(window_ == NULL);  // FIXME(loicm) Multiple windows are not supported yet.
  window_ = new QHybrisLegacyWindow(window, static_cast<QHybrisLegacyScreen*>(screen_));
  window_->requestActivateWindow();
  return window_;
}
