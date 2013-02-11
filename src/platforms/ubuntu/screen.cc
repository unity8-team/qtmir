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

const int kDefaultGridUnit = 18;

QUbuntuScreen::QUbuntuScreen() {
  // Retrieve units from the environment.
  int gridUnit = kDefaultGridUnit;
  QByteArray gridUnitString = qgetenv("GRID_UNIT_PX");
  if (!gridUnitString.isEmpty()) {
    bool ok;
    gridUnit = gridUnitString.toInt(&ok);
    if (!ok) {
      gridUnit = kDefaultGridUnit;
    }
  }
  gridUnit_ = gridUnit;
  densityPixel_ = qRound(gridUnit * (1.0 / 8.0));
  DLOG("grid unit is %d", gridUnit);
  DLOG("density pixel is %d", densityPixel_);

  // Compute menubar strut.
  // FIXME(loicm) Hard-coded to 3 grid units minus 2 density independent pixels for now.
  struct { int left; int right; int top; int bottom; } strut = {
    0, 0, toGridUnit(3) - toDensityPixel(2), 0
  };
  DLOG("menu bar height is %d pixels", strut.top);

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
      strut.left, strut.top, kScreenWidth - strut.left - strut.right,
      kScreenHeight - strut.top - strut.bottom);

  DLOG("QUbuntuScreen::QUbuntuScreen (this=%p)", this);

  // Set the default orientation based on the initial screen dimmensions.
  nativeOrientation_ = kScreenWidth >= kScreenHeight ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

QUbuntuScreen::~QUbuntuScreen() {
  DLOG("QUbuntuScreen::~QUbuntuScreen");
}
