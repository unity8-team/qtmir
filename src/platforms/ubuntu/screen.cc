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

#include "screen.h"
#include "base/logging.h"
#include <ubuntu/application/ui/ubuntu_application_ui.h>

// FIXME(loicm) There is no way to get the strut from a system session. Values are hard-coded for
//     the phone right now, 59 corresponds to 3 grid units minus 2 density independent pixels.
static const struct { int left; int right; int top; int bottom; } kStrut = { 0, 0, 59, 0 };

QUbuntuScreen::QUbuntuScreen() {
  // Get screen resolution.
  ubuntu_application_ui_physical_display_info info;
  ubuntu_application_ui_create_display_info(&info, 0);
  const int kScreenWidth = ubuntu_application_ui_query_horizontal_resolution(info);
  const int kScreenHeight = ubuntu_application_ui_query_vertical_resolution(info);
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  DLOG("screen resolution: %dx%d", kScreenWidth, kScreenHeight);
  ubuntu_application_ui_destroy_display_info(info);

  // Store geometries.
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);
  availableGeometry_ = QRect(
      kStrut.left, kStrut.top, kScreenWidth - kStrut.left - kStrut.right,
      kScreenHeight - kStrut.top - kStrut.bottom);

  DLOG("QUbuntuScreen::QUbuntuScreen (this=%p)", this);
}

QUbuntuScreen::~QUbuntuScreen() {
  DLOG("QUbuntuScreen::~QUbuntuScreen");
}
