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

#include "screen.h"
#include "base/logging.h"
#include <surface_flinger/surface_flinger_compatibility_layer.h>

QUbuntuLegacyScreen::QUbuntuLegacyScreen() {
  const int kScreenWidth = sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  const int kScreenHeight = sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);
  ASSERT((sfClient_ = sf_client_create_full(false)) != NULL);
  DLOG("QUbuntuLegacyScreen::QUbuntuLegacyScreen (this=%p)", this);
}

QUbuntuLegacyScreen::~QUbuntuLegacyScreen() {
  DLOG("QUbuntuLegacyScreen::~QUbuntuLegacyScreen");
  // FIXME(loicm) Invalid because the struct is forward declarated, we need a way to clean the
  //     handle correctly.
  // delete sfClient_;
}
