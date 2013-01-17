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

QUbuntuLegacyIntegration::QUbuntuLegacyIntegration()
    : window_(NULL)
    , screen_(new QUbuntuLegacyScreen())
    , input_(NULL) {
  screenAdded(screen_);
  if (qEnvironmentVariableIsEmpty("QTUBUNTU_NO_INPUT")) {
    // Input initialization needs to be delayed in order to avoid crashes in the input stack.
    int delay = kInputDelay;
    QByteArray swapIntervalString = qgetenv("QTUBUNTU_INPUT_DELAY");
    if (!swapIntervalString.isEmpty()) {
      bool valid;
      delay = qMax(1, swapIntervalString.toInt(&valid));
      if (!valid)
        delay = kInputDelay;
    }
    DLOG("delaying input initialization for %d ms", delay);
    QTimer::singleShot(delay, this, SLOT(initInput()));
  }
  DLOG("QUbuntuLegacyIntegration::QUbuntuLegacyIntegration (this=%p)", this);
}

QUbuntuLegacyIntegration::~QUbuntuLegacyIntegration() {
  DLOG("QUbuntuLegacyIntegration::~QUbuntuLegacyIntegration");
  delete input_;
  delete inputContext_;
  delete screen_;
}

void QUbuntuLegacyIntegration::initInput() {
  DLOG("QUbuntuLegacyIntegration::initInput (this=%p)", this);
  input_ = new QUbuntuLegacyInput(this);
  inputContext_ = QPlatformInputContextFactory::create();
}

QPlatformWindow* QUbuntuLegacyIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QUbuntuLegacyIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QUbuntuLegacyIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QUbuntuLegacyIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QUbuntuLegacyIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  ASSERT(window_ == NULL);  // FIXME(loicm) Multiple windows are not supported yet.
  window_ = new QUbuntuLegacyWindow(window, static_cast<QUbuntuLegacyScreen*>(screen_));
  window_->requestActivateWindow();
  return window_;
}
