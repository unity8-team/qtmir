// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application.h"
#include "application_manager.h"
#include "logging.h"
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

Application::Application(
    DesktopData* desktopData, qint64 pid, Application::State state, int timerId)
    : desktopData_(desktopData)
    , pid_(pid)
    , state_(state)
    , timerId_(timerId) {
  DASSERT(desktopData != NULL);
  DLOG("Application::Application (this=%p, desktopData=%p, pid=%lld, state=%d, timerId=%d)",
       this, desktopData, pid, static_cast<int>(state), timerId);
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

Application::State Application::state() const {
  return state_;
}

void Application::setState(Application::State state) {
  DLOG("Application::setState (this=%p, state=%d)", this, static_cast<int>(state));
  if (state_ != state) {
    state_ = state;
    emit stateChanged();
  }
}
