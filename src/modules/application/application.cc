// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application.h"
#include "logging.h"

Application::Application(const char* name, const char* comment, const char* icon, int handle)
    : name_(name)
    , comment_(comment)
    , icon_(icon)
    , handle_(handle)
    , focused_(false) {
  DLOG("Application::Application (this=%p, name='%s', comment='%s', icon='%s', handle=%d)",
       this, name, comment, icon, handle);
}

Application::~Application() {
  DLOG("Application::~Application");
}

void Application::setFocused(bool focused) {
  DLOG("Application::setFocused (this=%p, focused=%d)", this, focused);
  if (focused_ != focused) {
    focused_ = focused;
    emit focusedChanged();
  }
}
