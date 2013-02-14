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

#include "application_window.h"
#include "application_manager.h"
#include "logging.h"

ApplicationWindow::ApplicationWindow(QObject* parent)
    : QObject(parent)
    , role_(static_cast<int>(ApplicationManager::Default))
    , opaque_(0) {
  DLOG("ApplicationWindow::ApplicationWindow (this=%p)", this);
}

ApplicationWindow::~ApplicationWindow() {
  DLOG("ApplicationWindow::~ApplicationWindow");
}

void ApplicationWindow::setRole(int role) {
  DLOG("ApplicationWindow::setRole (this=%p, role=%d)", this, role);
  if (role_ != role) {
    role_ = role;
    emit roleChanged();
  }
}

void ApplicationWindow::setOpaque(bool opaque) {
  DLOG("ApplicationWindow::setOpaque (this=%p, opaque=%d)", this, static_cast<int>(opaque));
  if (opaque_ != static_cast<int>(opaque)) {
    opaque_ = static_cast<int>(opaque);
    emit opaqueChanged();
  }
}
