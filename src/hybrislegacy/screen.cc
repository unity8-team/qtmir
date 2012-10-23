// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "screen.h"
#include "base/logging.h"
#include <surface_flinger/surface_flinger_compatibility_layer.h>

QHybrisScreen::QHybrisScreen() {
  const int kScreenWidth = sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  const int kScreenHeight = sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);
  ASSERT((sfClient_ = sf_client_create_full(false)) != NULL);
  DLOG("QHybrisScreen::QHybrisScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
  // FIXME(loicm) Invalid because the struct is forward declarated, we need a way to clean the
  //     handle correctly.
  // delete sfClient_;
}
