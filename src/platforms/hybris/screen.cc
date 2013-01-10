// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "screen.h"
#include "base/logging.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

// FIXME(loicm) There is no way to get the strut from a system session. Values are hard-coded for
//     the phone right now, 59 corresponds to 3 grid units minus 2 density independent pixels.
static const struct { int left; int right; int top; int bottom; } kStrut = { 0, 0, 59, 0 };

QHybrisScreen::QHybrisScreen() {
  // Get screen resolution.
  ubuntu_application_ui_physical_display_info info;
  ubuntu_application_ui_create_display_info(&info, 0);
  const int kScreenWidth = ubuntu_application_ui_query_horizontal_resolution(info);
  const int kScreenHeight = ubuntu_application_ui_query_vertical_resolution(info);
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  ubuntu_application_ui_destroy_display_info(info);

  // Store geometries.
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);
  availableGeometry_ = QRect(
      kStrut.left, kStrut.top, kScreenWidth - kStrut.left - kStrut.right,
      kScreenHeight - kStrut.top - kStrut.bottom);

  DLOG("QHybrisScreen::QHybrisScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
}
