// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "input.h"
#include "integration.h"
#include "base/logging.h"
#include <qpa/qplatformwindow.h>

static void hybrisEventCallback(Event* event, void* context) {
  DLOG("hybrisEventCallback (event=%p, context=%p)", event, context);
  QHybrisInput* input = static_cast<QHybrisInput*>(context);
  if (!input->stopping_.testAndSetRelease(1, 1)) {
    // FIXME(loicm) We need to be able to retrieve the window from an event in order to support
    //     multiple surfaces.
    QPlatformWindow* window = static_cast<QHybrisIntegration*>(
        input->integration())->platformWindow();
    if (window) {
      input->handleEvent(window->window(), event);
    }
  }
}

QHybrisInput::QHybrisInput(QHybrisIntegration* integration)
    : QHybrisBaseInput(integration, MAX_POINTER_COUNT)
    , stopping_(0) {
  config_.enable_touch_point_visualization = false;
  config_.default_layer_for_touch_point_visualization = 1;
  listener_.on_new_event = hybrisEventCallback;
  listener_.context = this;
  DLOG("initializing input stack");
  android_input_stack_initialize(&listener_, &config_);
  DLOG("starting input stack");
  android_input_stack_start();
  DLOG("QHybrisInput::QHybrisInput (this=%p, integration=%p)", this, integration);
}

QHybrisInput::~QHybrisInput() {
  DLOG("QHybrisInput::~QHybrisInput");
  stopping_.fetchAndStoreRelease(1);
  DLOG("stopping input stack");
  android_input_stack_stop();
  DLOG("shutting down input stack");
  android_input_stack_shutdown();
}
