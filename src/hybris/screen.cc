// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "screen.h"
#include "base/logging.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

QHybrisScreen::QHybrisScreen() {
  // FIXME(loicm) Ubuntu application UI doesn't provide the screen size.
  const int kScreenWidth = 720;
  const int kScreenHeight = 1280;
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);
  DLOG("QHybrisScreen::QHybrisScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
}
