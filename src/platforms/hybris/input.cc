// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "input.h"
#include "integration.h"
#include "base/logging.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

QHybrisInput::QHybrisInput(QHybrisIntegration* integration)
    : QHybrisBaseInput(integration, UBUNTU_APPLICATION_UI_INPUT_EVENT_MAX_POINTER_COUNT) {
  DLOG("QHybrisInput::QHybrisInput (this=%p integration=%p)", this, integration);
}

QHybrisInput::~QHybrisInput() {
  DLOG("QHybrisInput::~QHybrisInput");
}
