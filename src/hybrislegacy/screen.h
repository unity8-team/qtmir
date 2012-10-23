// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISSCREEN_H
#define QHYBRISSCREEN_H

#include "base/screen.h"

struct SfClient;

class QHybrisScreen : public QHybrisBaseScreen {
 public:
  QHybrisScreen();
  ~QHybrisScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }

  // New methods.
  SfClient* sfClient() const { return sfClient_; }

 private:
  QRect geometry_;
  SfClient* sfClient_;
};

#endif  // QHYBRISSCREEN_H
