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

#include "application.h"
#include "application_manager.h"
#include "logging.h"

Application::Application(
    DesktopData* desktopData, qint64 pid, Application::Stage stage, Application::State state,
    int timerId)
    : desktopData_(desktopData)
    , pid_(pid)
    , stage_(stage)
    , state_(state)
    , fullscreen_(false)
    , timerId_(timerId) {
  DASSERT(desktopData != NULL);
  DLOG("Application::Application (this=%p, desktopData=%p, pid=%lld, stage=%d, state=%d, "
       "timerId=%d)", this, desktopData, pid, static_cast<int>(stage), static_cast<int>(state),
       timerId);
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

Application::Stage Application::stage() const {
  return stage_;
}

Application::State Application::state() const {
  return state_;
}

bool Application::fullscreen() const {
  return fullscreen_;
}

void Application::setStage(Application::Stage stage) {
  DLOG("Application::setStage (this=%p, stage=%d)", this, static_cast<int>(stage));
  if (stage_ != stage) {
    stage_ = stage;
    emit stageChanged();
  }
}

void Application::setState(Application::State state) {
  DLOG("Application::setState (this=%p, state=%d)", this, static_cast<int>(state));
  if (state_ != state) {
    state_ = state;
    emit stateChanged();
  }
}

void Application::setFullscreen(bool fullscreen) {
  DLOG("Application::setFullscreen (this=%p, fullscreen=%s)", this, fullscreen ? "yes" : "no");
  if (fullscreen_ != fullscreen) {
    fullscreen_ = fullscreen;
    emit fullscreenChanged();
  }
}
