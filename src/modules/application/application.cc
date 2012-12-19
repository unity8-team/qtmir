// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application.h"
#include "application_manager.h"
#include "logging.h"
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

Application::Application(DesktopData* desktopData, qint64 pid)
    : desktopData_(desktopData)
    , pid_(pid)
    , focused_(false) {
  DASSERT(desktopData != NULL);
  DLOG("Application::Application (this=%p, desktopData=%p, pid=%lld)", this, desktopData, pid);
}

Application::~Application() {
  DLOG("Application::~Application");
  delete desktopData_;
}

QString Application::desktopFile() const {
  return desktopData_->file();
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

qint64 Application::handle() const {
  return pid_;
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
