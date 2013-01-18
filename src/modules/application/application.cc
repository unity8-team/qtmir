// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "application.h"
#include "application_manager.h"
#include "logging.h"

Application::Application(DesktopData* desktopData, QProcess* process, int handle)
    : desktopData_(desktopData)
    , process_(process)
    , handle_(handle) {
  DASSERT(desktopData != NULL);
  DLOG("Application::Application (this=%p, desktopData=%p, handle=%d)", this, desktopData, handle);
}

Application::~Application() {
  DLOG("Application::~Application");
  delete desktopData_;
  delete process_;
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

int Application::handle() const {
  return handle_;
}
