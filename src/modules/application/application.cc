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
#include "desktopdata.h"
#include "logging.h"

using namespace unity::shell::application;

Application::Application(
    DesktopData* desktopData, qint64 pid, Application::Stage stage, Application::State state,
    int timerId)
    : ApplicationInfoInterface(desktopData->appId())
    , desktopData_(desktopData)
    , pid_(pid)
    , stage_(stage)
    , state_(state)
    , focused_(false)
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

QString Application::appId() const {
  return desktopData_->appId();
}

QString Application::name() const {
  return desktopData_->name();
}

QString Application::comment() const {
  return desktopData_->comment();
}

QUrl Application::icon() const {
  return desktopData_->icon();
}

QString Application::exec() const {
  return desktopData_->exec();
}

qint64 Application::pid() const {
  return pid_;
}

Application::Stage Application::stage() const {
  return stage_;
}

Application::State Application::state() const {
  return state_;
}

bool Application::focused() const {
  return focused_;
}

bool Application::fullscreen() const {
  return fullscreen_;
}

void Application::setStage(Application::Stage stage) {
  DLOG("Application::setStage (this=%p, stage=%d)", this, static_cast<int>(stage));
  if (stage_ != stage) {
    stage_ = stage;
    emit stageChanged(stage);
  }
}

void Application::setState(Application::State state) {
  DLOG("Application::setState (this=%p, state=%d)", this, static_cast<int>(state));
  if (state_ != state) {
    state_ = state;
    emit stateChanged(state);
  }
}

void Application::setFocused(bool focused) {
    DLOG("Application::setFocused (this=%p, focused=%d)", this, static_cast<int>(focused));
    if (focused_ != focused) {
      focused_ = focused;
      emit focusedChanged(focused);
    }
}

void Application::setFullscreen(bool fullscreen) {
  DLOG("Application::setFullscreen (this=%p, fullscreen=%s)", this, fullscreen ? "yes" : "no");
  if (fullscreen_ != fullscreen) {
    fullscreen_ = fullscreen;
    emit fullscreenChanged(fullscreen);
  }
}
