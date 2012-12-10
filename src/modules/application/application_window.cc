// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application_window.h"
#include "application_manager.h"
#include "logging.h"

ApplicationWindow::ApplicationWindow(QObject* parent)
    : QObject(parent)
    , role_(static_cast<int>(ApplicationManager::Default)) {
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
