// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

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
