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
#include <qpa/qplatformwindow.h>

static void eventCallback(Event* event, void* context) {
  DLOG("eventCallback (event=%p, context=%p)", event, context);
  QUbuntuLegacyInput* input = static_cast<QUbuntuLegacyInput*>(context);
  if (!input->stopping_.testAndSetRelease(1, 1)) {
    // FIXME(loicm) We need to be able to retrieve the window from an event in order to support
    //     multiple surfaces.
    QPlatformWindow* window = static_cast<QUbuntuLegacyIntegration*>(
        input->integration())->platformWindow();
    if (window) {
      input->postEvent(window->window(), event);
    }
  }
}

QUbuntuLegacyInput::QUbuntuLegacyInput(QUbuntuLegacyIntegration* integration)
    : QUbuntuBaseInput(integration)
    , stopping_(0) {
  config_.enable_touch_point_visualization = false;
  config_.default_layer_for_touch_point_visualization = 1;
  listener_.on_new_event = eventCallback;
  listener_.context = this;
  DLOG("initializing input stack");
  android_input_stack_initialize(&listener_, &config_);
  DLOG("starting input stack");
  android_input_stack_start();
  DLOG("QUbuntuLegacyInput::QUbuntuLegacyInput (this=%p, integration=%p)", this, integration);
}

QUbuntuLegacyInput::~QUbuntuLegacyInput() {
  DLOG("QUbuntuLegacyInput::~QUbuntuLegacyInput");
  stopping_.fetchAndStoreRelease(1);
  DLOG("stopping input stack");
  android_input_stack_stop();
  DLOG("shutting down input stack");
  android_input_stack_shutdown();
}
