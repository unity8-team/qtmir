// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application_window.h"
#include "logging.h"

ApplicationWindow::ApplicationWindow(QObject* parent)
    : QObject(parent)
    , role_(ApplicationWindow::Main) {
  DLOG("ApplicationWindow::ApplicationWindow (this=%p)", this);
}

ApplicationWindow::~ApplicationWindow() {
  DLOG("ApplicationWindow::~ApplicationWindow");
}

void ApplicationWindow::setRole(ApplicationWindow::Role role) {
  DLOG("ApplicationWindow::setRole (this=%p, role=%d)", this, role);
  if (role_ != role) {
    role_ = role;
    emit roleChanged();
  }
}
