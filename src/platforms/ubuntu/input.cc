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

#include "input.h"
#include "integration.h"
#include "base/logging.h"
#include <ubuntu/application/ui/input/event.h>

QUbuntuInput::QUbuntuInput(QUbuntuIntegration* integration)
    : QUbuntuBaseInput(integration, UBUNTU_APPLICATION_UI_INPUT_EVENT_MAX_POINTER_COUNT)
    , sessionType_(0) {
  DLOG("QUbuntuInput::QUbuntuInput (this=%p integration=%p)", this, integration);
}

QUbuntuInput::~QUbuntuInput() {
  DLOG("QUbuntuInput::~QUbuntuInput");
}

void QUbuntuInput::handleTouchEvent(
    QWindow* window, ulong timestamp, QTouchDevice* device,
    const QList<struct QWindowSystemInterface::TouchPoint> &points) {
  DLOG("QUbuntuInput::handleTouchEvent (this=%p, window=%p, timestamp=%lu, device=%p)",
       this, window, timestamp, device);
  if (sessionType_ != 1) {
    QUbuntuBaseInput::handleTouchEvent(window, timestamp, device, points);
  } else {
    // Ubuntu platform API creates an input handler per window. Since system sessions have
    // fullscreen input handlers, the last created window has an input handler that takes precedence
    // over the others. Because of that, only the last created window receives touch input. In order
    // to fix that issue for system sessions, we pass the NULL pointer to the Qt handler as window
    // argument so that it pushes the event to the window that's located at the touch point.
    QUbuntuBaseInput::handleTouchEvent(NULL, timestamp, device, points);
  }
}

void QUbuntuInput::setSessionType(uint sessionType) {
  DLOG("QUbuntuInput::setSessionType (this=%p, window=%u)", this, sessionType);
  sessionType_ = sessionType;
}
