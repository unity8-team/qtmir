// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application.h"
#include "application_manager.h"
#include "logging.h"

Application::Application(DesktopData* desktopData, QProcess* process, int handle)
    : desktopData_(desktopData)
    , process_(process)
    , handle_(handle)
    , focused_(false) {
  DASSERT(desktopData != NULL);
  DLOG("Application::Application (this=%p, desktopData=%p, handle=%d)", this, desktopData, handle);
}

Application::~Application() {
  DLOG("Application::~Application");
  delete desktopData_;
  delete process_;
}

QString Application::name() const {
  return desktopData_->name();
}

QString Application::comment() const {
  return desktopData_->comment();
}

QString Application::icon() const {
  return desktopData_->icon();
}

QString Application::exec() const {
  return desktopData_->exec();
}

int Application::handle() const {
  return handle_;
}

bool Application::focused() const {
  return focused_;
}

void Application::setFocused(bool focused) {
  DLOG("Application::setFocused (this=%p, focused=%d)", this, focused);
  if (focused_ != focused) {
    focused_ = focused;
    emit focusedChanged();
  }
}
