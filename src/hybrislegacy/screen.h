// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISLEGACYSCREEN_H
#define QHYBRISLEGACYSCREEN_H

#include "base/screen.h"

struct SfClient;

class QHybrisLegacyScreen : public QHybrisBaseScreen {
 public:
  QHybrisLegacyScreen();
  ~QHybrisLegacyScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }

  // New methods.
  SfClient* sfClient() const { return sfClient_; }

 private:
  QRect geometry_;
  SfClient* sfClient_;
};

#endif  // QHYBRISLEGACYSCREEN_H
