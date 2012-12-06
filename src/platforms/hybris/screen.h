// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISSCREEN_H
#define QHYBRISSCREEN_H

#include "base/screen.h"

class QHybrisScreen : public QHybrisBaseScreen {
 public:
  QHybrisScreen();
  ~QHybrisScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }
  QRect availableGeometry() const { return availableGeometry_; }

 private:
  QRect geometry_;
  QRect availableGeometry_;
};

#endif  // QHYBRISSCREEN_H
