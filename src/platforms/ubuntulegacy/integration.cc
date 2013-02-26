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
#include "window.h"
#include "input.h"
#include "base/clipboard.h"
#include "base/logging.h"
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>
#include <QTimer>

// That value seems to work on every systems and applications tested so far.
static const int kInputDelay = 1000;

QUbuntuLegacyIntegration::QUbuntuLegacyIntegration()
    : window_(NULL)
    , screen_(new QUbuntuLegacyScreen())
    , input_(NULL)
    , clipboard_(new QUbuntuBaseClipboard()) {
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
  delete clipboard_;
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
